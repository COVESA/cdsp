# Base image for Node.js
FROM node:22.5.1

#########################
# GENERAL CONFIGURATION #
#########################

# HANDLER_TYPE can be initialized with two different values: `realmdb` or `iotdb`
ENV HANDLER_TYPE=${HANDLER_TYPE}

#########################
# REALMDB CONFIGURATION #
#########################

# VERSION must be 0 or a positive integer. This is used for versioning the RealmDB configuration schema.
ENV VERSION_REALMDB_SCHEMA=${VERSION_REALMDB_SCHEMA}

# Access to ATLAS Cloud instance
ENV REALMDB_APP_ID=${REALMDB_APP_ID}
ENV REALMDB_API_KEY=${REALMDB_API_KEY}

#######################
# IOTDB CONFIGURATION #
#######################

# Access to iotdb-service
ENV IOTDB_HOST: ${IOTDB_HOST}
ENV IOTDB_PORT: ${IOTDB_PORT}
ENV IOTDB_USER: ${IOTDB_USER}
ENV IOTDB_PASSWORD: ${IOTDB_PASSWORD}
ENV IOTDB_FETCH_SIZE: ${IOTDB_FETCH_SIZE}

# Set working directory inside the container
WORKDIR /app

COPY . ./

# Install dependencies, including workspaces
RUN npm install --production

# Expose the necessary port
EXPOSE 8080

# Command to run the WebSocket server
CMD ["node", "router/src/websocket-server.js"]