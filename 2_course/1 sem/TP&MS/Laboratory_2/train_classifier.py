import string
import math
import os
import pandas as pd
from collections import defaultdict
from sklearn.model_selection import train_test_split

class SpamFilter:
    def __init__(self):
        self.class_prior = {}  # Вероятность класса
        self.word_likelihood = defaultdict(lambda: defaultdict(float))  # Вероятность слова в классе
        self.word_set = set()  # Набор уникальных слов
        self.labels_set = set()  # Набор меток
        self.label_counts = defaultdict(int)  # Счётчик email по меткам

    def clean_text(self, text):
        if pd.isna(text):
            text = ""
        text = str(text).translate(str.maketrans("", "", string.punctuation)).lower()
        return text.split()  

    def train(self, documents, classes):
        if len(documents) != len(classes):
            raise ValueError("Число писем и меток не совпадает")

        word_freq = defaultdict(lambda: defaultdict(int))  

        for doc, cls in zip(documents, classes):
            self.labels_set.add(cls)
            self.label_counts[cls] += 1
            words = self.clean_text(doc)
            self.word_set.update(words)
            for word in set(words):  
                word_freq[cls][word] += 1

        total_docs = len(documents)
        for cls in self.labels_set:
            self.class_prior[cls] = self.label_counts[cls] / total_docs

        # Применяем сглаживание Лапласа
        for cls in self.labels_set:
            doc_count = self.label_counts[cls]
            for word in self.word_set:
                self.word_likelihood[cls][word] = (word_freq[cls][word] + 1) / (doc_count + 2)

    def classify(self, documents):
        results = []
        for doc in documents:
            words = self.clean_text(doc)
            log_scores = {}
            for cls in self.labels_set:
                score = math.log(self.class_prior[cls])
                for word in set(words):  
                    if word in self.word_set:
                        score += math.log(self.word_likelihood[cls][word])
                log_scores[cls] = score

            spam_score = log_scores.get(1, float('-inf'))
            ham_score = log_scores.get(0, float('-inf'))
            result = 1 if spam_score > ham_score else 0
            results.append(result)

        return results

def read_csv_data(file_path):
    """
    Читает данные из CSV-файла Enron Spam.
    Возвращает списки текстов и меток.
    """
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"Файл {file_path} не найден. Проверьте расположение enron_spam_data.csv.")
    
    data = pd.read_csv(file_path)
    print(f"Обработано {len(data)} записей из CSV.")
    
    data['content'] = data['Subject'].fillna('') + ' ' + data['Message'].fillna('')
    data['class_label'] = data['Spam/Ham'].map({'spam': 1, 'ham': 0})
    
    if data['class_label'].isnull().any():
        raise ValueError("Найдены некорректные метки в 'Spam/Ham'.")
    
    contents = data['content'].tolist()
    labels = data['class_label'].tolist()
    
    print(f"Статистика: {sum(labels)} спама ({sum(labels)/len(labels)*100:.1f}%), {len(labels) - sum(labels)} не спама.")
    
    return contents, labels

def assess_performance(true_labels, pred_labels):
    """
    Оценивает точность, чувствительность и специфичность.
    """
    true_pos, true_neg, false_pos, false_neg = 0, 0, 0, 0
    for true, pred in zip(true_labels, pred_labels):
        if true == 1 and pred == 1:
            true_pos += 1
        elif true == 0 and pred == 0:
            true_neg += 1
        elif true == 0 and pred == 1:
            false_pos += 1
        elif true == 1 and pred == 0:
            false_neg += 1
    
    total = true_pos + true_neg + false_pos + false_neg
    acc = (true_pos + true_neg) / total if total > 0 else 0
    sens = true_pos / (true_pos + false_neg) if (true_pos + false_neg) > 0 else 0
    spec = true_neg / (true_neg + false_pos) if (true_neg + false_pos) > 0 else 0
    
    return acc, sens, spec

if __name__ == "__main__":
    # Путь к файлу
    file_path = "enron_spam_data.csv"  # Укажите реальный путь, если файл не в текущей директории
    
    docs, classes = read_csv_data(file_path)
    
    # Разделение на обучающую и тестовую выборки
    train_docs, test_docs, train_classes, test_classes = train_test_split(
        docs, classes, test_size=0.2, random_state=42, stratify=classes
    )
    
    # Обучение модели
    classifier = SpamFilter()
    classifier.train(train_docs, train_classes)
    
    # Предсказание
    predictions = classifier.classify(test_docs)
    
    # Оценка результатов
    accuracy, sensitivity, specificity = assess_performance(test_classes, predictions)
    print(f"Результаты: Точность={accuracy:.3f}, Чувствительность={sensitivity:.3f}, Специфичность={specificity:.3f}")