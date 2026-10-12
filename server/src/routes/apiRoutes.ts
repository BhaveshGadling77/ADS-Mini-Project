import { FastifyInstance } from 'fastify';
import { CompressionController } from '../controllers/compressionController.js';

export async function apiRoutes(fastify: FastifyInstance) {
  fastify.get('/health', async () => {
    return { status: 'ok', timestamp: new Date().toISOString() };
  });

  fastify.post('/api/compress', CompressionController.compress);
  fastify.post('/api/decompress', CompressionController.decompress);
  fastify.post('/api/compare', CompressionController.compare);
  fastify.get('/api/jobs/:jobId', CompressionController.getJob);
  fastify.get('/api/jobs/:jobId/download', CompressionController.downloadJob);
  fastify.get('/api/history', CompressionController.getHistory);
}
