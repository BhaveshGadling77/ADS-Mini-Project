import Fastify from 'fastify';
import cors from '@fastify/cors';
import multipart from '@fastify/multipart';
import fastifyStatic from '@fastify/static';
import path from 'path';
import { existsSync } from 'fs';
import { fileURLToPath } from 'url';
import { config } from './config/index.js';
import { StorageService } from './services/storageService.js';
import { apiRoutes } from './routes/apiRoutes.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

async function buildServer() {
  const fastify = Fastify({
    logger: true,
  });

  await fastify.register(cors, {
    origin: true,
    credentials: true,
  });

  await fastify.register(multipart, {
    limits: {
      fileSize: config.maxFileSize,
    },
  });

  await StorageService.init();

  await fastify.register(apiRoutes);

  // Serve static web build if present
  const webDist = path.resolve(__dirname, '../../web/dist');
  if (existsSync(webDist)) {
    await fastify.register(fastifyStatic, {
      root: webDist,
      prefix: '/',
    });

    fastify.setNotFoundHandler((req, reply) => {
      if (req.raw.url && req.raw.url.startsWith('/api')) {
        return reply.status(404).send({ error: 'Endpoint not found' });
      }
      return reply.sendFile('index.html');
    });
  }

  fastify.setErrorHandler((error: any, _request, reply) => {
    if (error.code === 'FST_REQ_FILE_TOO_LARGE') {
      return reply.status(413).send({
        success: false,
        error: `File is too large. Maximum supported file size is ${config.maxFileSize / (1024 * 1024)} MB.`,
      });
    }
    const statusCode = error.statusCode || 500;
    reply.status(statusCode).send({
      success: false,
      error: error.message || 'Internal server error',
    });
  });

  return fastify;
}

async function main() {
  const app = await buildServer();
  try {
    const address = await app.listen({ port: config.port, host: config.host });
    console.log(`🚀 ADS Compression Backend running at ${address}`);
  } catch (err) {
    app.log.error(err);
    process.exit(1);
  }
}

main();
