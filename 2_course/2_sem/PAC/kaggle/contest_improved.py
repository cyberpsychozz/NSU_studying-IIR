import argparse
import os
import shutil
from pathlib import Path

os.environ.setdefault("YOLO_CONFIG_DIR", str(Path.cwd().resolve()))

import pandas as pd
from PIL import Image
from sklearn.model_selection import train_test_split
from ultralytics import RTDETR


CLASS_NAMES = [
    "Aortic enlargement",
    "Atelectasis",
    "Calcification",
    "Cardiomegaly",
    "Consolidation",
    "ILD",
    "Infiltration",
    "Lung Opacity",
    "Nodule/Mass",
    "Other lesion",
    "Pleural effusion",
    "Pleural thickening",
    "Pneumothorax",
    "Pulmonary fibrosis",
]
NO_FINDING = 14


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--data-dir", type=Path, default=Path("amia-public-challenge-2026"))
    parser.add_argument("--work-dir", type=Path, default=Path("yolo_data_v2"))
    parser.add_argument("--project", type=Path, default=Path("training_runs"))
    parser.add_argument("--run-name", type=str, default="amia_rtdetr_l_960_v1")
    parser.add_argument("--model", type=str, default="rtdetr-l.pt")
    parser.add_argument("--epochs", type=int, default=30)
    parser.add_argument("--imgsz", type=int, default=640)
    parser.add_argument("--batch", type=int, default=3)
    parser.add_argument("--patience", type=int, default=15)
    parser.add_argument("--workers", type=int, default=4)
    parser.add_argument("--device", type=str, default="0")
    parser.add_argument("--val-size", type=float, default=0.15)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--cache", type=str, default="disk")
    parser.add_argument("--conf", type=float, default=0.03)
    parser.add_argument("--iou", type=float, default=0.50)
    parser.add_argument("--max-det", type=int, default=100)
    return parser.parse_args()


def key(x):
    return Path(str(x)).stem


def safe_link(src: Path, dst: Path):
    try:
        os.link(src, dst)
    except OSError:
        shutil.copy2(src, dst)


def build_image_table(train_df: pd.DataFrame) -> pd.DataFrame:
    positive = train_df[train_df["class_id"] != NO_FINDING].copy()
    class_freq = positive.groupby("class_id")["image_id"].nunique().to_dict()

    image_rows = []
    for image_id, group in train_df.groupby("image_id", sort=False):
        pos_classes = sorted(set(group.loc[group["class_id"] != NO_FINDING, "class_id"].tolist()))
        if not pos_classes:
            stratify_key = "neg"
        else:
            rarest = min(pos_classes, key=lambda cls: (class_freq[cls], cls))
            stratify_key = f"pos_{rarest}"
        image_rows.append({"image_id": image_id, "stratify_key": stratify_key})

    return pd.DataFrame(image_rows)


def write_label_file(rows: pd.DataFrame, out_path: Path, orig_h: float, orig_w: float):
    lines = []
    for row in rows.itertuples(index=False):
        x1 = max(0.0, min(float(row.x_min), orig_w - 1))
        y1 = max(0.0, min(float(row.y_min), orig_h - 1))
        x2 = max(0.0, min(float(row.x_max), orig_w - 1))
        y2 = max(0.0, min(float(row.y_max), orig_h - 1))

        if x2 <= x1 or y2 <= y1:
            continue

        xc = ((x1 + x2) / 2.0) / orig_w
        yc = ((y1 + y2) / 2.0) / orig_h
        bw = (x2 - x1) / orig_w
        bh = (y2 - y1) / orig_h
        lines.append(f"{int(row.class_id)} {xc:.6f} {yc:.6f} {bw:.6f} {bh:.6f}")

    out_path.write_text("\n".join(lines), encoding="utf-8")


def make_dataset(args):
    train_csv = args.data_dir / "train.csv"
    img_size_csv = args.data_dir / "img_size.csv"
    train_img_dir = args.data_dir / "train" / "train"

    train = pd.read_csv(train_csv)
    sizes = pd.read_csv(img_size_csv)
    size_lookup = {
        key(row.image_id): (float(row.dim0), float(row.dim1))
        for row in sizes.itertuples(index=False)
    }

    image_table = build_image_table(train)
    train_ids, val_ids = train_test_split(
        image_table["image_id"],
        test_size=args.val_size,
        random_state=args.seed,
        stratify=image_table["stratify_key"],
    )
    split_lookup = {image_id: "train" for image_id in train_ids}
    split_lookup.update({image_id: "val" for image_id in val_ids})

    positive = train[train["class_id"] != NO_FINDING].copy()

    if args.work_dir.exists():
        shutil.rmtree(args.work_dir)

    for split in ("train", "val"):
        (args.work_dir / "images" / split).mkdir(parents=True, exist_ok=True)
        (args.work_dir / "labels" / split).mkdir(parents=True, exist_ok=True)

    for image_id in image_table["image_id"]:
        image_key = key(image_id)
        split = split_lookup[image_id]
        src = train_img_dir / f"{image_key}.png"
        dst = args.work_dir / "images" / split / src.name
        safe_link(src, dst)

        orig_h, orig_w = size_lookup[image_key]
        rows = positive[positive["image_id"] == image_id]
        label_path = args.work_dir / "labels" / split / f"{image_key}.txt"
        write_label_file(rows, label_path, orig_h, orig_w)

    (args.work_dir / "split_manifest.csv").write_text(
        pd.DataFrame(
            [{"image_id": image_id, "split": split} for image_id, split in split_lookup.items()]
        ).sort_values(["split", "image_id"]).to_csv(index=False),
        encoding="utf-8",
    )

    yaml_lines = [
        f"path: {args.work_dir.resolve()}",
        "train: images/train",
        "val: images/val",
        "names:",
    ]
    yaml_lines.extend([f"  {i}: {name}" for i, name in enumerate(CLASS_NAMES)])
    (args.work_dir / "data.yaml").write_text("\n".join(yaml_lines) + "\n", encoding="utf-8")

    return args.work_dir / "data.yaml", size_lookup


def resolve_best_path(project: Path, run_name: str) -> Path:
    expected = project / "detect" / run_name / "weights" / "best.pt"
    if expected.exists():
        return expected

    candidates = sorted(project.glob(f"**/{run_name}/weights/best.pt"))
    if not candidates:
        raise FileNotFoundError(f"Could not find best.pt for run '{run_name}' inside {project}")
    return candidates[-1]


def train_model(args, yaml_path: Path) -> Path:
    model = RTDETR(args.model)
    model.train(
        data=str(yaml_path),
        epochs=args.epochs,
        imgsz=args.imgsz,
        batch=args.batch,
        project=str(args.project),
        name=args.run_name,
        exist_ok=True,
        patience=args.patience,
        device=args.device,
        workers=args.workers,
        cache=args.cache,
        optimizer="auto",
        cos_lr=True,
        amp=True,
        plots=True,
        hsv_h=0.0,
        hsv_s=0.0,
        hsv_v=0.0,
        degrees=2.0,
        translate=0.02,
        scale=0.10,
        shear=0.0,
        perspective=0.0,
        flipud=0.0,
        fliplr=0.0,
        mosaic=0.0,
        mixup=0.0,
        copy_paste=0.0,
        close_mosaic=0,
        auto_augment=None,
        erasing=0.0,
    )
    return resolve_best_path(args.project, args.run_name)


def make_submission(args, best_path: Path, size_lookup):
    test_img_dir = args.data_dir / "test" / "test"
    sample_sub = args.data_dir / "sample_submission.csv"
    out_csv = args.project / "detect" / args.run_name / "submission.csv"
    out_csv.parent.mkdir(parents=True, exist_ok=True)

    model = RTDETR(str(best_path))
    sample = pd.read_csv(sample_sub)
    id_col = sample.columns[0]

    rows = []
    for raw_id in sample[id_col].astype(str):
        image_id = key(raw_id)
        img_path = test_img_dir / f"{image_id}.png"

        img_w, img_h = Image.open(img_path).size
        orig_h, orig_w = size_lookup[image_id]

        pred = model.predict(
            str(img_path),
            imgsz=args.imgsz,
            conf=args.conf,
            iou=args.iou,
            max_det=args.max_det,
            verbose=False,
        )[0]

        tokens = []
        if pred.boxes is not None and len(pred.boxes) > 0:
            boxes = pred.boxes.xyxy.cpu().numpy()
            scores = pred.boxes.conf.cpu().numpy()
            classes = pred.boxes.cls.cpu().numpy().astype(int)

            for box, score, cls in zip(boxes, scores, classes):
                x1, y1, x2, y2 = box
                x1 *= orig_w / img_w
                x2 *= orig_w / img_w
                y1 *= orig_h / img_h
                y2 *= orig_h / img_h
                tokens.extend(
                    [
                        str(cls),
                        f"{score:.6f}",
                        f"{x1:.1f}",
                        f"{y1:.1f}",
                        f"{x2:.1f}",
                        f"{y2:.1f}",
                    ]
                )

        pred_string = " ".join(tokens) if tokens else f"{NO_FINDING} 1 0 0 1 1"
        rows.append({id_col: raw_id, "PredictionString": pred_string})

    pd.DataFrame(rows).to_csv(out_csv, index=False)
    return out_csv


def main():
    args = parse_args()
    args.project.mkdir(parents=True, exist_ok=True)

    yaml_path, size_lookup = make_dataset(args)
    best_path = train_model(args, yaml_path)
    submission_path = make_submission(args, best_path, size_lookup)

    print(f"Best weights: {best_path}")
    print(f"Submission:   {submission_path}")


if __name__ == "__main__":
    main()
