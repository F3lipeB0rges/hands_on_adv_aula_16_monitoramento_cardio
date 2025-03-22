import streamlit as st
import requests
import pandas as pd
import plotly.express as px
import time

API_URL = "http://192.168.1.186:5000/dados"

st.title("Monitor de ECG - AD8232 com ESP32")

# Função para buscar os dados do servidor com cache
@st.cache_data(ttl=1)  # Atualiza os dados a cada 1 segundo
def get_ecg_data():
    response = requests.get(API_URL)
    if response.status_code == 200:
        return response.json()
    return []

# Criar o espaço para o gráfico
chart_placeholder = st.empty()

while True:
    ecg_data = get_ecg_data()

    if ecg_data:
        df = pd.DataFrame({"ECG": ecg_data})
        fig = px.line(df, y="ECG", title="Sinal de ECG em Tempo Real")
        unique_key = f"ecg_chart_{int(time.time())}"
        chart_placeholder.plotly_chart(fig, use_container_width=True,key=unique_key)
    time.sleep(1)  # Atualiza a cada 1 segundo
