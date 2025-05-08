FROM node:22.5.1

WORKDIR /app

COPY package.json .
COPY tsconfig.json .
COPY handlers ./handlers
COPY utils ./utils
COPY router ./router

# Install dependencies, including workspaces
RUN npm install

# Transpile TypeScript to JavaScript
RUN npm run build

# Remove devDependencies to reduce the final image size
RUN npm prune --omit=dev

# Expose the necessary port
EXPOSE 8080

# Command to run the WebSocket server
CMD ["node", "./dist/router/src/websocket-server.js"]