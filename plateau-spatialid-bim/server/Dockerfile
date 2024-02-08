FROM node:18.18-slim

WORKDIR /app

COPY build/src /app

COPY package.json package*.json ./

RUN npm ci --only=production

RUN groupadd -r app && useradd --no-log-init -r -g app app

ENV TZ="Asia/Tokyo"

USER app

CMD [ "node", "--enable-source-maps", "main.js" ]
