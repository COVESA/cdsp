FROM python:3.12-slim
# Disable Python output buffering to show logs immediately
ENV PYTHONUNBUFFERED=1

WORKDIR /app
COPY requirements.txt .
RUN pip3 install -r requirements.txt

COPY rl-bridge.py .

ENTRYPOINT ["python", "./rl-bridge.py"]
