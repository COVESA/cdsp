FROM node:22.5.1

WORKDIR /app

COPY package.json .
COPY handlers ./handlers
COPY utils ./utils
COPY router ./router

# Install dependencies, including workspaces
RUN npm install --production

# Expose the necessary port
EXPOSE 8080

# Command to run the WebSocket server
CMD ["node", "router/src/websocket-server.js"]
