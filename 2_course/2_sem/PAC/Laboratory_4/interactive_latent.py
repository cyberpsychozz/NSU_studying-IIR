import torch
import torch.nn as nn
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import torchvision
import torchvision.transforms as transforms
from torch.utils.data import DataLoader
import matplotlib
matplotlib.use('TkAgg')

DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")
OUT_DIR = "./outputs"
DATA_DIR = "./data"

CLASS_NAMES = ["T-shirt", "Trouser", "Pullover", "Dress", "Coat",
               "Sandal",  "Shirt",   "Sneaker",  "Bag",   "Ankle boot"]

class ConvAE(nn.Module):
    def __init__(self):
        super().__init__()
        # Encoder
        self.enc_conv = nn.Sequential(
            nn.Conv2d(1, 32, kernel_size=3, padding=1), 
            nn.ReLU(),
            nn.MaxPool2d(2, 2), 

            nn.Conv2d(32, 16, kernel_size=3, padding=1), 
            nn.ReLU(),
            nn.MaxPool2d(2, 2)
        )
        self.enc_fc = nn.Linear(16 * 7 * 7, 2) 

        # Decoder
        self.dec_fc = nn.Sequential(nn.Linear(2, 16 * 7 * 7), 
                                    nn.ReLU())
        self.dec_conv = nn.Sequential(
            nn.Upsample(scale_factor=2, mode='nearest'),
            nn.Conv2d(16, 16, kernel_size=3, padding=1), 
            nn.ReLU(),

            nn.Upsample(scale_factor=2, mode='nearest'), 
            nn.Conv2d(16, 1, kernel_size=3, padding=1), 
            nn.Sigmoid()
        )

    def forward(self, x):
        x = self.enc_conv(x)
        x = x.view(-1, 16 * 7 * 7)
        z = self.enc_fc(x)

        out = self.dec_fc(z)
        out = out.view(-1, 16, 7, 7)
        out = self.dec_conv(out)
        return out, z

    def encode(self, x):
        x = self.enc_conv(x)
        x = x.view(-1, 16 * 7 * 7)
        return self.enc_fc(x)

    def decode(self, z):
        out = self.dec_fc(z)
        out = out.view(-1, 16, 7, 7)
        return self.dec_conv(out)


ae2d = ConvAE()
ae2d.load_state_dict(torch.load(f"/home/cyberpsychoz/NSU_studying-IIR/2_course/2_sem/PAC/Laboratory_4/model_cnn_weights.pth", map_location=DEVICE))
ae2d.to(DEVICE)
ae2d.eval()
print("Model loaded!")

tf = transforms.ToTensor()
test_set = torchvision.datasets.FashionMNIST(DATA_DIR, train=False, download=True, transform=tf)
test_loader = DataLoader(test_set, batch_size=256, shuffle=False)

all_z, all_lbl = [], []
with torch.no_grad():
    for x, l in test_loader:
        all_z.append(ae2d.encode(x.to(DEVICE)).cpu().numpy())
        all_lbl.append(l.numpy())
all_z = np.concatenate(all_z)
all_lbl = np.concatenate(all_lbl)


fig = plt.figure(figsize=(15, 7))
fig.suptitle("Interactive Latent Space Decoder", fontsize=14, fontweight="bold")

ax_sc = fig.add_axes([0.04, 0.12, 0.54, 0.80])
ax_img = fig.add_axes([0.65, 0.30, 0.30, 0.55])
ax_s1 = fig.add_axes([0.65, 0.18, 0.30, 0.04])
ax_s2 = fig.add_axes([0.65, 0.10, 0.30, 0.04])

colors = plt.cm.tab10(np.linspace(0, 1, 10))
for c in range(10):
    m = all_lbl == c
    ax_sc.scatter(all_z[m, 0], all_z[m, 1], c=[colors[c]],
                  label=CLASS_NAMES[c], s=4, alpha=0.5)
ax_sc.legend(fontsize=8, markerscale=4, loc="upper right")
ax_sc.set(xlabel="z₁", ylabel="z₂", title="Click to decode")
ax_sc.grid(alpha=0.2)

cursor, = ax_sc.plot([], [], "r+", ms=14, mew=2.5, zorder=10)
img_disp = ax_img.imshow(np.zeros((28, 28)), cmap="gray", vmin=0, vmax=1)
ax_img.set_title("Decoded image")
ax_img.axis("off")

z1_min, z1_max = float(all_z[:, 0].min()), float(all_z[:, 0].max())
z2_min, z2_max = float(all_z[:, 1].min()), float(all_z[:, 1].max())

sl_z1 = Slider(ax_s1, "z₁", z1_min, z1_max, valinit=(z1_min+z1_max)/2, color="royalblue")
sl_z2 = Slider(ax_s2, "z₂", z2_min, z2_max, valinit=(z2_min+z2_max)/2, color="tomato")

def decode_and_show(z1, z2):
    z = torch.tensor([[z1, z2]], dtype=torch.float32).to(DEVICE)
    with torch.no_grad():
        img = ae2d.decode(z).cpu().squeeze().numpy()
    img_disp.set_data(img)
    cursor.set_data([z1], [z2])
    ax_img.set_title(f"z = ({z1:.2f}, {z2:.2f})")
    fig.canvas.draw_idle()

def on_click(event):
    if event.inaxes != ax_sc or event.xdata is None:
        return
    sl_z1.set_val(event.xdata)
    sl_z2.set_val(event.ydata)

def on_slider(_):
    decode_and_show(sl_z1.val, sl_z2.val)

sl_z1.on_changed(on_slider)
sl_z2.on_changed(on_slider)
fig.canvas.mpl_connect("button_press_event", on_click)

decode_and_show((z1_min+z1_max)/2, (z2_min+z2_max)/2)
plt.show()