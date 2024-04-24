FROM node:21-alpine AS base
FROM base AS development

WORKDIR /app

COPY package.json .
COPY package-lock.json .

RUN npm install

COPY . .
