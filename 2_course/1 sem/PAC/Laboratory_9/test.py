import pandas as pd
import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import RandomForestClassifier
from xgboost import XGBClassifier
from sklearn.neighbors import KNeighborsClassifier

def prepare_num(df):
    df_num = df.drop(['Sex', 'Embarked', 'Pclass'], axis=1)
    df_sex = pd.get_dummies(df['Sex'])
    df_emb = pd.get_dummies(df['Embarked'], prefix='Emb')
    df_pcl = pd.get_dummies(df['Pclass'], prefix='Pclass')

    df_num = pd.concat((df_num, df_sex, df_emb, df_pcl), axis=1)
    return df_num

df_main = pd.read_csv('data/train.csv')
df_test = pd.read_csv('data/test.csv')

df_prep_x = df_main.drop(['PassengerId', 'Survived', 'Name', 'Ticket', 'Cabin'], axis=1)
df_prep_x_tst = df_test.drop(['PassengerId', 'Name', 'Ticket', 'Cabin'], axis=1)
df_prep_y = df_main['Survived']

df_prep_x_num = prepare_num(df_prep_x)
df_prep_x_num_tst = prepare_num(df_prep_x_tst)

df_prep_x_num = df_prep_x_num.fillna(df_prep_x_num.median())
df_prep_x_num_tst = df_prep_x_num_tst.fillna(df_prep_x_num.median())

df_prep_x
