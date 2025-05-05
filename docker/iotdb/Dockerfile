# SPDX-FileCopyrightText: Copyright (c) 2024 Renesas Electronics
# SPDX-License-Identifier: MPL-2.0
#
# Dockerfile for the CDSP build of Apache IoTDB.
# ---------------------------------------------------------------------------

ARG IOTDB_VERSION=1.3.3
ARG IOTDB_IMAGE=apache/iotdb:${IOTDB_VERSION}-standalone

FROM ${IOTDB_IMAGE} AS cdsp-extras

ARG IOTDB_VERSION
ARG IOTDB_IMAGE

# UDF Data Quality library
ARG UDF_ARCHIVE_NAME=apache-iotdb-${IOTDB_VERSION}-library-udf-bin.zip
ARG UDF_ARCHIVE_URL=https://archive.apache.org/dist/iotdb/${IOTDB_VERSION}/${UDF_ARCHIVE_NAME}
ARG UDF_LIB_PATH=apache-iotdb-${IOTDB_VERSION}-library-udf-bin/library-udf.jar
ARG UDF_REG_SCRIPT_NAME=register-UDF.sh
ARG UDF_REG_SCRIPT_PATH=apache-iotdb-${IOTDB_VERSION}-library-udf-bin/${UDF_REG_SCRIPT_NAME}
ARG SERVICE_HOSTNAME=iotdb-service

# General configuration
RUN apt update \
    && apt install unzip -y

# Add the optional IoTDB UDF Data Quality Library to the image.
# Note: to use the library functions the registration script must first be run in the running container.
WORKDIR ${IOTDB_HOME}/ext/udf
ADD ${UDF_ARCHIVE_URL} .
RUN unzip -j -o ${UDF_ARCHIVE_NAME} ${UDF_LIB_PATH} ${UDF_REG_SCRIPT_PATH} \
    && sed -i "s/^host=127.0.0.1/host=${SERVICE_HOSTNAME}/g" ${UDF_REG_SCRIPT_NAME} \
    && mv ${UDF_REG_SCRIPT_NAME} ${IOTDB_HOME}/sbin/ \
    && rm ${UDF_ARCHIVE_NAME}

WORKDIR ${IOTDB_HOME}/conf
# Enable IoTDB REST API as it is used by the IoTDB Grafana Connector for queries
RUN echo "enable_rest_service=true" >> iotdb-system.properties

# Set the entry point path to the IoTDB sbin directory where the IoTDB admin scripts are.
# This mimics the upstream image.
WORKDIR ${IOTDB_HOME}/sbin
