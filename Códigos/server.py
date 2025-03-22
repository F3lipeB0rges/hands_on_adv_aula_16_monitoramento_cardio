from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

dados_ecg = []

@app.route('/dados', methods=['POST'])
def receber_dados():
    global dados_ecg
    json_data = request.get_json()
    if "ecg" in json_data:
        dados_ecg.append(json_data["ecg"])
        if len(dados_ecg) > 100:  # Mantém os últimos 100 valores
            dados_ecg.pop(0)
    return jsonify({"status": "ok"})

@app.route('/dados', methods=['GET'])
def enviar_dados():
    return jsonify(dados_ecg)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
