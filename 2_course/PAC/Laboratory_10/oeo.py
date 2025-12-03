import numpy as np
import matplotlib.pyplot as plt 
import torchvision

# Initializing the transform for the dataset
transform = torchvision.transforms.Compose([
                    torchvision.transforms.ToTensor(),
                    torchvision.transforms.Normalize((0.5), (0.5))
                                            ])

# Downloading the MNIST dataset
train_dataset = torchvision.datasets.MNIST(
    root="MNIST/train", train=True,
    transform=torchvision.transforms.ToTensor(),
    download=False)

test_dataset = torchvision.datasets.MNIST(
    root="MNIST/test", train=False,
    transform=torchvision.transforms.ToTensor(),
    download=False)

test1 = train_dataset[0]
# (Img, label)

def encode_label(j):
    # 5 -> [[0], [0], [0], [0], [0], [1], [0], [0], [0], [0]]
    e = np.zeros((10,1))
    e[j] = 1.0
    return e

def shape_data(data):
    features = [np.reshape(x[0][0].numpy(), (784,1)) for x in data]
    labels = [encode_label(y[1]) for y in data]
    return zip(features, labels)

train = shape_data(train_dataset)
test = shape_data(test_dataset)

train = list(train)
test = list(test)

def average_digit(data, digit):
    filtered_data = [x[0] for x in data if np.argmax(x[1]) == digit]
    filtered_array = np.asarray(filtered_data)
    return np.average(filtered_array, axis=0)

class MyClassifier:
    def __init__(self, W, norm, b = 0.0):
        self.W = W
        self.b = b
        self.norm = norm
    
    def predict(self, x):
        x_norm = np.linalg.norm(x)
        if x_norm == 0:
            return 0.0
        cos_sim = (np.dot(self.W, x)[0][0] / (self.norm * x_norm)) + self.b
        return 1.0 if cos_sim >= 0 else 0.0
    

avgs = {i: average_digit(train, i) for i in range(10)}
weights = {i: np.transpose(avgs[i]) for i in range(10)} 
norms = {i: np.linalg.norm(weights[i]) for i in range(10)}

accuracies = []
for i in range(10):
    model = MyClassifier(weights[i], norms[i], b=-30)  
    TP, TN, FP, FN = 0, 0, 0, 0
    for digit, label_onehot in test:
        label = np.argmax(label_onehot)
        prediction = model.predict(digit)
        if label == i:
            if prediction == 1:
                TP += 1
            else:
                FN += 1 
        else:
            if prediction == 0:
                TN += 1
            else:
                FP += 1
    acc = (TP + TN) / (TP + TN + FP + FN) if (TP + TN + FP + FN) > 0 else 0
    accuracies.append(acc)

class DigitsClassifier:
    def __init__(self, weights, norms, accuracies, b=-0.05):
        self.classifiers = [MyClassifier(weights[i], norms[i], b) for i in range(10)]  
        self.accuracies = accuracies
    
    def predict(self, x):
        
        predictions = [self.accuracies[i] * c.predict(x) for i, c in enumerate(self.classifiers)]
        if max(predictions) == 0:  
            raw_sims = [np.dot(weights[i], x)[0][0] / (norms[i] * np.linalg.norm(x)) for i in range(10)]
            max_idx = np.argmax(raw_sims)
        else:
            max_idx = np.argmax(predictions)
        answer = np.zeros((10, 1))
        answer[max_idx] = 1
        return answer

classifier = DigitsClassifier(weights, norms, accuracies, b = -30)

per_class = []
for target_class in range(10):
    TP = FP = FN = TN = 0
    for digit, label_onehot in test:
        true_label = np.argmax(label_onehot)
        pred_onehot = classifier.predict(digit)
        pred_label = np.argmax(pred_onehot)
        
        if true_label == target_class:
            if pred_label == target_class:
                TP += 1
            else:
                FN += 1
        else:
            if pred_label == target_class:
                FP += 1
            else:
                TN += 1
    precision = TP / (TP + FP) if (TP + FP) > 0 else 0
    recall = TP / (TP + FN) if (TP + FN) > 0 else 0
    per_class.append((precision, recall))


precisions, recalls = zip(*per_class)
macro_precision = np.mean(precisions)
macro_recall = np.mean(recalls)

print(f"Macro Precision: {macro_precision:.4f}")
print(f"Macro Recall: {macro_recall:.4f}")