
# Bitshrink - Web Frontend (React + Vite + Tailwind CSS)
# Multi-stage Dockerfile for the Web UI Client

# 
# Stage 1: Build React + Vite Web Application
 
FROM node:22-bookworm-slim AS web-builder

WORKDIR /workspace/shared
COPY shared/ ./

WORKDIR /workspace/web
COPY web/package*.json ./
RUN npm ci

COPY web/ ./
ARG VITE_API_BASE_URL=""
ENV VITE_API_BASE_URL=${VITE_API_BASE_URL}
RUN npm run build

# 
# Stage 2: Minimal Production Web Server (Nginx)
# 
FROM nginx:alpine AS runner

# Copy built static SPA assets to Nginx html folder
COPY --from=web-builder /workspace/web/dist /usr/share/nginx/html

# Copy dynamic Nginx template for automatic backend routing
COPY docker/web/nginx.conf.template /etc/nginx/templates/default.conf.template

ENV BACKEND_HOST=server \
    BACKEND_PORT=3000

EXPOSE 80

HEALTHCHECK --interval=15s --timeout=3s --start-period=5s --retries=3 \
  CMD wget --quiet --tries=1 --spider http://127.0.0.1/ || exit 1

CMD ["nginx", "-g", "daemon off;"]
