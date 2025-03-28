FROM python:3.9

# Disable Python output buffering to show logs immediately
ENV PYTHONUNBUFFERED=1

WORKDIR /app
COPY requirements.txt .
COPY rl-bridge.py .

RUN pip3 install -r requirements.txt

ENTRYPOINT ["python", "./rl-bridge.py"]
