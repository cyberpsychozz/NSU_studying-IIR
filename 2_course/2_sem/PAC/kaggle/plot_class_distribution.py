import argparse
import os
from pathlib import Path

os.environ.setdefault("MPLCONFIGDIR", str(Path.cwd() / ".matplotlib_cache"))

import matplotlib.pyplot as plt
import pandas as pd


NO_FINDING = 14


def parse_args():
    parser = argparse.ArgumentParser(description="Plot class distribution for AMIA challenge train.csv")
    parser.add_argument(
        "--train-csv",
        type=Path,
        default=Path("amia-public-challenge-2026/train.csv"),
        help="Path to train.csv",
    )
    parser.add_argument(
        "--out-dir",
        type=Path,
        default=Path("class_distribution_plots"),
        help="Directory for plots and summary tables",
    )
    return parser.parse_args()


def save_barh(df: pd.DataFrame, x_col: str, title: str, xlabel: str, out_path: Path, color: str):
    plot_df = df.sort_values(x_col, ascending=True)

    fig_height = max(6, 0.38 * len(plot_df) + 1.5)
    fig, ax = plt.subplots(figsize=(11, fig_height))
    bars = ax.barh(plot_df["class_name"], plot_df[x_col], color=color)

    ax.set_title(title, fontsize=15, pad=14)
    ax.set_xlabel(xlabel)
    ax.set_ylabel("")
    ax.grid(axis="x", alpha=0.25)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)

    max_value = float(plot_df[x_col].max()) if len(plot_df) else 0.0
    label_offset = max_value * 0.01 if max_value else 1.0
    for bar in bars:
        width = bar.get_width()
        ax.text(
            width + label_offset,
            bar.get_y() + bar.get_height() / 2,
            f"{int(width):,}".replace(",", " "),
            va="center",
            fontsize=9,
        )

    fig.tight_layout()
    fig.savefig(out_path, dpi=180, bbox_inches="tight")
    plt.close(fig)


def build_summary(train: pd.DataFrame) -> pd.DataFrame:
    row_counts = (
        train.groupby(["class_id", "class_name"], as_index=False)
        .size()
        .rename(columns={"size": "annotation_rows"})
    )
    image_counts = (
        train.groupby(["class_id", "class_name"], as_index=False)["image_id"]
        .nunique()
        .rename(columns={"image_id": "unique_images"})
    )

    summary = row_counts.merge(image_counts, on=["class_id", "class_name"], how="outer")
    total_rows = summary["annotation_rows"].sum()
    total_images = train["image_id"].nunique()
    summary["row_percent"] = summary["annotation_rows"] / total_rows * 100
    summary["image_percent"] = summary["unique_images"] / total_images * 100
    return summary.sort_values("class_id").reset_index(drop=True)


def main():
    args = parse_args()
    args.out_dir.mkdir(parents=True, exist_ok=True)

    train = pd.read_csv(args.train_csv)
    summary = build_summary(train)
    positive_summary = summary[summary["class_id"] != NO_FINDING].copy()

    summary.to_csv(args.out_dir / "class_distribution_summary.csv", index=False)
    positive_summary.to_csv(args.out_dir / "positive_class_distribution_summary.csv", index=False)

    save_barh(
        summary,
        "annotation_rows",
        "Class distribution by annotation rows",
        "Rows in train.csv",
        args.out_dir / "class_rows_all.png",
        "#4C78A8",
    )
    save_barh(
        summary,
        "unique_images",
        "Class distribution by unique images",
        "Unique images",
        args.out_dir / "class_images_all.png",
        "#59A14F",
    )
    save_barh(
        positive_summary,
        "annotation_rows",
        "Positive class distribution by bounding boxes",
        "Bounding boxes",
        args.out_dir / "class_boxes_positive.png",
        "#F28E2B",
    )
    save_barh(
        positive_summary,
        "unique_images",
        "Positive class distribution by unique images",
        "Unique images",
        args.out_dir / "class_images_positive.png",
        "#B07AA1",
    )

    total_images = train["image_id"].nunique()
    no_finding_images = int(train.loc[train["class_id"] == NO_FINDING, "image_id"].nunique())
    positive_images = total_images - no_finding_images

    fig, ax = plt.subplots(figsize=(7, 5))
    bars = ax.bar(
        ["Positive findings", "No finding"],
        [positive_images, no_finding_images],
        color=["#E15759", "#76B7B2"],
    )
    ax.set_title("Images with findings vs no finding", fontsize=15, pad=14)
    ax.set_ylabel("Unique images")
    ax.grid(axis="y", alpha=0.25)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    for bar in bars:
        height = bar.get_height()
        ax.text(
            bar.get_x() + bar.get_width() / 2,
            height + max(total_images * 0.01, 1),
            f"{int(height):,}".replace(",", " "),
            ha="center",
            va="bottom",
            fontsize=10,
        )
    fig.tight_layout()
    fig.savefig(args.out_dir / "positive_vs_no_finding_images.png", dpi=180, bbox_inches="tight")
    plt.close(fig)

    print(f"Saved plots and summaries to: {args.out_dir.resolve()}")


if __name__ == "__main__":
    main()
