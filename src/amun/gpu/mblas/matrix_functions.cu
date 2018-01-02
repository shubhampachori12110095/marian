#include "common/histories.h"
#include "gpu/mblas/matrix_functions.h"
#include "gpu/mblas/handles.h"
#include "gpu/decoder/enc_out_gpu.h"

using namespace std;

namespace amunmt {
namespace GPU {
namespace mblas {

thread_local CudaStreamHandler CudaStreamHandler::instance_;
thread_local CublasHandler CublasHandler::instance_;


Matrix& Swap(Matrix& Out, Matrix& In) {
  Out.swap(In);
  return Out;
}

__global__ void gMean(MatrixWrapper<float> out,
                      const MatrixWrapper<float> in,
                      const VectorWrapper<uint> sentenceLengths)
{
  // out = batches * states
  // in = max sentence length * states * 1 * batches
  // mapping = max length * batches

  int id = threadIdx.x + blockIdx.x * blockDim.x;
  //printf("id = %d in = %lu %lu %lu %lu = %lu %lu \n", id, in.dim(0), in.dim(1), in.dim(2), in.dim(3), in.size(), sizeof(in));

  if (id < out.size()) {
    uint indices[SHAPE_SIZE];
    out.id2Indices(id, indices);
    //printf("%d -> %lu %lu %lu %lu \n", id, indices[0], indices[1], indices[2], indices[3]);

    size_t batch = indices[0];
    size_t state = indices[1];

    float sum = 0.0f;
    int counter = 0;
    for (size_t row = 0; row < in.dim(0); ++row) {
      bool isWord = row < sentenceLengths[batch];
      //printf("batch=%lu startMapInd=%lu  mapOffset=%lu -> %d \n", batch, startMapInd, mapOffset, isWord);
      if (isWord) {
        sum += in(row, state, 0, batch);
        ++counter;
      }
    }

    sum /= (float) counter;
    out[id] = sum;
  }
}

void Mean(Matrix& Out,
          const Matrix& In,
          const mblas::Vector<uint> &sentenceLengths)
{
  assert(Out.dim(2) == 1);
  assert(Out.dim(3) == 1);
  assert(Out.dim(0) == In.dim(3));
  assert(Out.dim(1) == In.dim(1));

  // mean of each ROW
  size_t batchNum = Out.dim(0) * Out.dim(2) * Out.dim(3);
  size_t stateLength = Out.dim(1);
  size_t sentenceLength = (In.dim(0) * In.dim(2) * In.dim(3)) / batchNum;

  MatrixWrapper<float> outWrap(Out);
  MatrixWrapper<float> inWrap(In);
  //cerr << "outWrap=" << outWrap.Debug() << endl;

  VectorWrapper<uint> sentenceLengthsWrap(sentenceLengths);

  uint size = outWrap.size();
  uint threads = std::min((uint)MAX_THREADS, size);
  uint blocks =  (size / threads) + ((size % threads == 0) ?  0 : 1);

  gMean<<<blocks, threads, 0, CudaStreamHandler::GetStream()>>>
    (outWrap, inWrap, sentenceLengthsWrap);

}

__global__ void gWeightedMean(MatrixWrapper<float> out,
                              const MatrixWrapper<float> weights,
                              const MatrixWrapper<float> in,
                              const VectorWrapper<size_t> hypo2Batch
                              )
{
  int numHypos = weights.dim(0);
  int states = in.dim(1);
  int srcLen = weights.dim(1);

  int id = threadIdx.x + blockIdx.x * blockDim.x;
  if (id < numHypos * states) {
    int hypoInd = id / states;
    int batchInd = hypo2Batch[hypoInd];
    int stateInd = id % states;
    //printf("hypoInd=%d batchInd=%d stateInd=%d \n", hypoInd, batchInd, stateInd);

    float sum = 0.0f;
    for (uint i = 0; i < srcLen; ++i) {
      sum += weights(hypoInd, i, 0, 0) * in(i, stateInd, 0, batchInd);
    }

    out[id] = sum;
  }
}

void WeightedMean(Matrix& Out,const Matrix& Weights, const Matrix& In, const mblas::Vector<size_t>& hypo2Batch)
{
  int numHypos = Weights.dim(0);
  int states = In.dim(1);

  Out.NewSize(numHypos, states);

  MatrixWrapper<float> outWrap(Out);
  MatrixWrapper<float> weightsWrap(Weights);
  MatrixWrapper<float> inWrap(In);
  VectorWrapper<size_t> hypo2BatchWrap(hypo2Batch);

  uint size = Out.size();
  uint nThreads = std::min((uint) MAX_THREADS, (uint)size);
  uint nBlocks =  (size / nThreads) + ((size % nThreads == 0) ?  0 : 1);

  gWeightedMean<<<nBlocks, nThreads, 0, CudaStreamHandler::GetStream()>>>
    (outWrap, weightsWrap, inWrap, hypo2BatchWrap);
  /*
  cerr << "nBlocks=" << nBlocks << endl;

  cerr << "Out=" << outWrap.Debug() << endl;
  cerr << "Weights=" << weightsWrap.Debug() << endl;
  cerr << "In=" << inWrap.Debug() << endl;
  cerr << "mapping=" << mapping.size() << endl;
  for (size_t i = 0; i < mapping.size(); ++i) {
    cerr << mapping[i] << " ";
  }
  cerr << endl << endl;
  */
}

Matrix& Transpose(Matrix& Out, const Matrix& In) {
  size_t m = In.dim(0);
  size_t n = In.dim(1);

  Out.NewSize(n, m);

  float alpha = 1.0;
  float beta  = 0.0;

  HANDLE_ERROR_CUBLAS(cublasSgeam(CublasHandler::GetHandle(), CUBLAS_OP_T, CUBLAS_OP_T, m, n, &alpha, In.data(), n,
				&beta, In.data(), n, Out.data(), m));

  return Out;
}

Matrix& Transpose(Matrix& Out) {
  thread_local Matrix Temp;
  Transpose(Temp, Out);
  Swap(Out, Temp);
  return Out;
}

Matrix& Concat(Matrix& Out, const Matrix& In) {
  size_t oldSize = Out.size();
  Out.Resize(Out.dim(0) + In.dim(0), Out.dim(1));

  mblas::copy(In.data(), In.size(), Out.data() + oldSize, cudaMemcpyDeviceToDevice);

  return Out;
}

Matrix& Copy(Matrix& Out, const Matrix& In) {
  Out.NewSize(In.dim(0), In.dim(1), In.dim(2), In.dim(3));

  mblas::copy(In.data(), In.size(), Out.data(), cudaMemcpyDeviceToDevice);

  return Out;
}

__global__ void gPasteRows(MatrixWrapper<float> out,
                          const MatrixWrapper<float> in,
                          int rowNo, int colNo)
{
  int inRows = in.dim(0);
  int inCols = in.dim(1);

  int id = threadIdx.x + blockIdx.x * blockDim.x;
  if (id < inRows * inCols) {
    int outCols = out.dim(1);

    int inRow = id / inCols;
    int inCol = id % inCols;

    //out[outID] = in[id];
    out(rowNo, inCol + colNo, 0, inRow) = in(inRow, inCol, 0, 0);
  }
}

void PasteRows(Matrix& Out, const Matrix& In, const size_t rowNo, size_t colNo)
{
  MatrixWrapper<float> outWrap(Out);
  MatrixWrapper<float> inWrap(In);

  uint size = In.size();
  uint nThreads = std::min((uint) MAX_THREADS, (uint)size);
  uint nBlocks =  (size / nThreads) + ((size % nThreads == 0) ?  0 : 1);

  gPasteRows<<<nBlocks, nThreads, 0, CudaStreamHandler::GetStream()>>>
    (outWrap, inWrap, rowNo, colNo);

}

Matrix& PasteRow(Matrix& Out,
                 const Matrix& In,
                 const size_t r, const size_t c)
{
  size_t start = r * Out.dim(1) + c;

  mblas::copy(In.data(), In.size(), Out.data() + start, cudaMemcpyDeviceToDevice);

  return Out;
}

Matrix& CopyRow(Matrix& Out,
                const Matrix& In,
                const size_t r, const size_t c) {
  size_t length = In.dim(1) - c;
  Out.NewSize(1, length);
  size_t start = r * In.dim(1) + c;
  //size_t end   = start + length;

  //mblas::copy(In.begin() + start, In.begin() + end, Out.begin());
  mblas::copy(In.data() + start, length , Out.data(), cudaMemcpyDeviceToDevice);

  return Out;
}

__global__ void gCopyRows(MatrixWrapper<float> out,
                          const MatrixWrapper<float> in,
                          const VectorWrapper<uint> indicesWrap)
{
  int id = threadIdx.x + blockIdx.x * blockDim.x;

  if (id < out.size()) {
	  uint dim[SHAPE_SIZE];
	  out.id2Indices(id, dim);

	  size_t indicesInd = dim[0];
	  size_t inRow =indicesWrap[indicesInd];

      out(indicesInd, dim[1], 0, 0) = in(inRow, dim[1], 0, 0);

  }
}

Matrix& CopyRows(Matrix& Out,
                 const Matrix& In,
                 const mblas::Vector<uint>& indices)
{
  assert(In.dim(1) == Out.dim(1));
  assert(Out.dim(0) == indices.size());

  assert(In.dim(2) == 1);
  assert(In.dim(3) == 1);
  assert(Out.dim(2) == 1);
  assert(Out.dim(3) == 1);

  /*
  cerr << "Out=" << Out.Debug(0) << endl;
  cerr << "In=" << In.Debug(0) << endl;
  cerr << "indices=" << Debug(indices, 2) << endl;
  cerr << endl;
  */

  size_t size = Out.size();

  size_t numPairs = indices.size();

  MatrixWrapper<float> outWrap(Out);
  const MatrixWrapper<float> inWrap(In);
  const VectorWrapper<uint> indicesWrap(indices);
  //cerr << "size=" << size << endl;

  uint threads = std::min((uint) MAX_THREADS, (uint)size);
  uint blocks = size / threads + ((size % threads == 0) ?  0 : 1);

  gCopyRows<<<blocks, threads, 0, CudaStreamHandler::GetStream()>>>
    (outWrap, inWrap, indicesWrap);

  return Out;
}


Matrix& Assemble(Matrix& Out,
                 const Matrix& In,
                 const mblas::Vector<uint>& indices) {
  Out.NewSize(indices.size(), In.dim(1));
  //cerr << "Assemble=" << Out.Debug() << " " << In.Debug() << indices.size() << endl;

  CopyRows(Out, In, indices);
  return Out;
}

__global__ void gSlice(MatrixWrapper<float> out,
                      const MatrixWrapper<float> in,
                       size_t n, size_t dim)
{
  size_t row = blockIdx.x;

  size_t inCol = threadIdx.x + dim * n;
  size_t outCol = threadIdx.x;

  while (outCol < out.dim(1)) {
    out(row, outCol, 0, 0) = in(row, inCol, 0, 0);

    inCol += blockDim.x;
    outCol += blockDim.x;
  }

}

Matrix& Slice(Matrix& Out,
              const Matrix& In,
              size_t n, size_t dim)
{
  assert(In.dim(2) == 1);
  assert(In.dim(3) == 1);

  Out.NewSize(In.dim(0), dim);

  MatrixWrapper<float> outWrap(Out);
  const MatrixWrapper<float> inWrap(In);

  /*
  cerr << "outWrap=" << outWrap.Debug() << endl;
  cerr << "inWrap=" << inWrap.Debug() << endl;
  cerr << "n=" << n << endl;
  cerr << "dim=" << dim << endl;
  cerr << endl;
  */

  uint threads = std::min((uint)MAX_THREADS, (uint)dim);
  uint blocks = In.dim(0);

  gSlice<<<blocks, threads, 0, CudaStreamHandler::GetStream()>>>
    (outWrap, inWrap, n, dim);
  return Out;
}

Matrix& Prod(cublasHandle_t handle, Matrix& C, const Matrix& A, const Matrix& B, bool transB)
{
  BEGIN_TIMER("Prod");
  assert((A.dim(2) == A.dim(3) == 1) || (B.dim(2) == B.dim(3) == 1));

  Matrix::value_type alpha = 1.0;
  Matrix::value_type beta = 0.0;

  size_t m = A.dim(0) * A.dim(2) * A.dim(3);
  size_t k = A.dim(1);
  size_t mOut = A.dim(0);
  size_t kOut = A.dim(1);

  /*
  if(transA) {
    std::swap(m, k);
    std::swap(mOut, kOut);
  }
  */
  size_t l = B.dim(0) * B.dim(2) * B.dim(3);
  size_t n = B.dim(1);
  size_t lOut = B.dim(0);
  size_t nOut = B.dim(1);
  if(transB) {
    std::swap(l, n);
    std::swap(lOut, nOut);
    }

  assert(k == l);

  size_t lda = A.dim(1);
  size_t ldb = B.dim(1);
  size_t ldc = transB ? B.dim(0) * B.dim(2) * B.dim(3) : B.dim(1);

  size_t dim2 = A.dim(2);
  if (transB) {
    // for GetAlignedSourceContext()
    assert((A.dim(2) == A.dim(3) == 1));
    C.NewSize(B.dim(2), nOut, 1, 1);
  }
  else {
    C.NewSize(mOut, nOut, A.dim(2) * B.dim(2), A.dim(3) * B.dim(3));
  }
  /*
  cerr << "C=" << C.Debug(0) << endl;
  cerr << "A=" << A.Debug(0) << endl;
  cerr << "B=" << B.Debug(0) << endl;
  cerr << "transB=" << transB << endl;
  cerr << m << " " << n << " " << k << endl;
  cerr << endl;
  */
  bool transA = false;
  cublasOperation_t opA = transA ? CUBLAS_OP_T : CUBLAS_OP_N;
  cublasOperation_t opB = transB ? CUBLAS_OP_T : CUBLAS_OP_N;

  HANDLE_ERROR_CUBLAS(cublasSgemm(handle, opB, opA,
                      n, m, k,
                      &alpha,
                      B.data(), ldb,
                      A.data(), lda,
                      &beta,
                      C.data(), ldc));
  PAUSE_TIMER("Prod");
  return C;
}

Matrix& Prod(Matrix& C, const Matrix& A, const Matrix& B,
             bool transB) {

  //std::cerr << "1C=" << C.Debug() << std::endl;
  //std::cerr << "1A=" << A.Debug() << std::endl;
  //std::cerr << "1B=" << B.Debug() << std::endl;

  Matrix &ret = Prod(CublasHandler::GetHandle(), C, A, B, transB);

  //std::cerr << "2C=" << C.Debug() << std::endl;
  return ret;
}

__global__ void gSoftMax(MatrixWrapper<float> out,
                         const VectorWrapper<size_t> hypo2BatchWrap,
                         const VectorWrapper<uint> sentenceLengthsWrap,
                         uint shareSize)
{
  extern __shared__ float _share[];

  size_t numHypos = out.dim(0);
  size_t maxLength = out.dim(1);

  int hypoInd =  blockIdx.x;
  int origSrcPos = threadIdx.x;

  while (hypoInd < numHypos) {
    VectorWrapper<float> _max(_share, shareSize);
    _max[origSrcPos] = out(hypoInd, origSrcPos, 0, 0);
    for (int tid = 0; tid < maxLength; tid += blockDim.x) {
      int srcPos = tid + origSrcPos;
      if (srcPos < maxLength) {
        float value = out(hypoInd, srcPos, 0, 0);

        int batch = hypo2BatchWrap[hypoInd];
        value *= srcPos < sentenceLengthsWrap[batch] ? 1 : 0;
        if (value > _max[origSrcPos]) {
          _max[origSrcPos] = value;
        }
      }
    }

    int len = blockDim.x;
    while (len != 1) {
      __syncthreads();

      int skip = (len + 1) >> 1;
      if (origSrcPos < (len >> 1)) {
        if(_max[origSrcPos + skip] > _max[origSrcPos])
          _max[origSrcPos] = _max[origSrcPos + skip];
      }
      len = (len + 1) >> 1;
    }
    __syncthreads();
    float max = _max[0];
    __syncthreads();

    //float* _sum = _share;// + blockDim.x;
    VectorWrapper<float> _sum(_share, shareSize);

    _sum[origSrcPos] = 0.0f;
    for (int tid = 0; tid < maxLength; tid += blockDim.x) {
      int srcPos = tid + origSrcPos;
      if (srcPos < maxLength) {
        out(hypoInd, srcPos, 0, 0) = __expf(out(hypoInd, srcPos, 0, 0) - max);

        int batch = hypo2BatchWrap[hypoInd];
        out(hypoInd, srcPos, 0, 0) *= srcPos < sentenceLengthsWrap[batch] ? 1 : 0; // sentencesMappingWrap(srcPos, batch, 0, 0);
        _sum[origSrcPos] += out(hypoInd, srcPos, 0, 0);
      }
    }

    __syncthreads();

    len = blockDim.x;
    while (len != 1) {
      __syncthreads();

      int skip = (len + 1) >> 1;
      if (origSrcPos < (len >> 1)) {
        _sum[origSrcPos] += _sum[origSrcPos + skip];
      }
      len = (len + 1) >> 1;
    }

    __syncthreads();

    for (int tid = 0; tid < maxLength; tid += blockDim.x) {
      int srcPos = tid + origSrcPos;
      if (srcPos < maxLength) {
        out(hypoInd, srcPos, 0, 0) /= _sum[0];
      }
    }
    __syncthreads();
    hypoInd += gridDim.x;
  }
}

Matrix& Softmax(Matrix& Out,
                const mblas::Vector<size_t>& hypo2Batch,
                const mblas::Vector<uint> &sentenceLengths,
                size_t batchSize)
{
  size_t maxLength = Out.dim(1);

  MatrixWrapper<float> outWrap(Out);
  const VectorWrapper<size_t> hypo2BatchWrap(hypo2Batch);
  const VectorWrapper<uint> sentenceLengthsWrap(sentenceLengths);

  int blocks = batchSize;
  int threads = std::min(MAX_THREADS, (int)maxLength);
  int shared = sizeof(float) * threads;

  gSoftMax<<<blocks, threads, shared, CudaStreamHandler::GetStream()>>>
    (outWrap, hypo2BatchWrap, sentenceLengthsWrap, threads);

  return Out;
}

__global__ void gLogSoftMax(MatrixWrapper<float> out, uint shareSize)
{
  extern __shared__ float _share[];

  size_t rows = out.dim(0);
  size_t cols = out.dim(1);

  int rowIdx =  blockIdx.x;

  while (rowIdx < rows) {
    //float* _max = _share;
    VectorWrapper<float> _max(_share, shareSize);

    _max[threadIdx.x] = out(rowIdx, threadIdx.x, 0, 0);
    for (int tid = 0; tid < cols; tid += blockDim.x) {
      int id = tid + threadIdx.x;
      if (id < cols) {
        const float &val = out(rowIdx, id, 0, 0);
        if (val > _max[threadIdx.x]) {
          _max[threadIdx.x] = val;
        }
      }
    }

    int len = blockDim.x;
    while (len != 1) {
      __syncthreads();

      int skip = (len + 1) >> 1;
      if (threadIdx.x < (len >> 1)) {
        if(_max[threadIdx.x + skip] > _max[threadIdx.x])
          _max[threadIdx.x] = _max[threadIdx.x + skip];
      }
      len = (len + 1) >> 1;
    }
    __syncthreads();
    float max = _max[0];
    __syncthreads();

    //float* _sum = _share;// + blockDim.x;
    VectorWrapper<float> _sum(_share, shareSize);

    _sum[threadIdx.x] = 0.0f;
    for (int tid = 0; tid < cols; tid += blockDim.x) {
      int id = tid + threadIdx.x;
      if (id < cols) {
        //row[id] = exp(row[id] - max);
        float &val = out(rowIdx, id, 0, 0);
        val = __expf(val - max);
        _sum[threadIdx.x] += val;
      }
    }

    len = blockDim.x;
    while (len != 1) {
      __syncthreads();

      int skip = (len + 1) >> 1;
      if (threadIdx.x < (len >> 1)) {
        _sum[threadIdx.x] += _sum[threadIdx.x + skip];
      }
      len = (len + 1) >> 1;
    }

    __syncthreads();

    for (int tid = 0; tid < cols; tid += blockDim.x) {
      int id = tid + threadIdx.x;
      if (id < cols) {
        //row[id] = log(row[id]/_sum[0]);
        float &val = out(rowIdx, id, 0, 0);
        val = __logf(val /_sum[0]);
      }
    }
    __syncthreads();
    rowIdx += gridDim.x;
  }
}


Matrix& LogSoftmax(Matrix& Out)
{
  MatrixWrapper<float> outWrap(Out);

  int blocks = std::min(MAX_BLOCKS, (int)Out.dim(0));
  int threads = std::min(MAX_THREADS, (int)Out.dim(1));
  int shared = sizeof(float) * threads;

  gLogSoftMax<<<blocks, threads, shared, CudaStreamHandler::GetStream()>>>
    (Out, threads);

  return Out;
}

__global__ void gSetColumn(MatrixWrapper<float> in, int noColumn, float value) {
  int n_rows = in.dim(0);

  int rowNumber = threadIdx.x  + blockDim.x * blockIdx.x;

  if (rowNumber < n_rows) {
    in(rowNumber, noColumn, 0, 0) = value;
  }
}

void SetColumn(Matrix& In, int noColumn, float value) {
  int nRows = In.dim(0);
  int nBlocks = nRows / MAX_THREADS + ((nRows % MAX_THREADS == 0) ?  0 : 1);
  int nThreads = std::min(MAX_THREADS, nRows);

  MatrixWrapper<float> inWrap(In);

  gSetColumn<<<nBlocks, nThreads, 0, mblas::CudaStreamHandler::GetStream()>>>
    (inWrap, noColumn, value);
}

__global__ void gFill(MatrixWrapper<float> in, float val) {
  int index = threadIdx.x + blockDim.x * blockIdx.x;
  if (index < in.size()) {
    in[index] = val;
  }
}

void Fill(Matrix& In, float value) {
  size_t size = In.size();

  if (value) {
    int nThreads = std::min(MAX_THREADS, (int)size);
    int nBlocks = (size / nThreads) + ((size % nThreads == 0) ? 0 : 1);

    MatrixWrapper<float> inWrap(In);

    gFill<<<nBlocks, nThreads, 0, CudaStreamHandler::GetStream()>>>
      (inWrap, value);
  }
  else {
    HANDLE_ERROR(cudaMemsetAsync(In.data(), 0, size * sizeof(float), CudaStreamHandler::GetStream()));
  }

}

__global__
void gMapMatrix(MatrixWrapper<float> in,
                const VectorWrapper<uint> sentenceLengthsWrap,
                int i)
{
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  if (tid < in.size()) {
    int numCols = in.dim(1);
    int batchIdx = tid / numCols;
    int col = tid % numCols;

    //in[tid] *= mappingWrap(i, batchIdx, 0, 0);
    in(batchIdx, col, 0, 0) *= (i < sentenceLengthsWrap[batchIdx] ? 1 : 0);
  }
}

void MapMatrix(Matrix& state,
              const mblas::Vector<uint> &sentenceLengths,
              size_t i)
{
  // blank out rows in the state matrix where the word position i does not exist
  // mapping is a concatenated array of 1 & 0 of each sentence in the batch to say whether word exists or not.

  int batchSize = state.dim(0);
  int stateLength = state.dim(1);

  int numThreads = std::min((int)state.size(), MAX_THREADS);
  int numBlocks = (state.size() / numThreads) + ((state.size() % numThreads == 0) ? 0 : 1);

  MatrixWrapper<float> stateWrap(state);
  VectorWrapper<uint> sentenceLengthsWrap(sentenceLengths);

  gMapMatrix<<<numBlocks, numThreads, 0, CudaStreamHandler::GetStream()>>>
    (stateWrap, sentenceLengthsWrap, i);

  /*
  cerr << "nBlocks=" << numBlocks << endl;
  cerr << "nThreads=" << numThreads << endl;
  cerr << "stateWrap=" << stateWrap.Debug() << endl;
  cerr << "mapping=" << Debug(mapping, 2) << endl;
  cerr << "i=" << i << endl;
  cerr << std::endl;

  HANDLE_ERROR(cudaDeviceSynchronize());
  */
}

__device__ uint getIndex(const dim3 &dim, const dim3 &val)
{
  uint ret = dim.x * val.x + dim.y * val.y + dim.z * val.z;
  return ret;
}


__global__ void gLNormalization(MatrixWrapper<float> out,
                                const MatrixWrapper<float> in,
                                const MatrixWrapper<float> alphaWrap,
                                const MatrixWrapper<float> betaWrap,
                                float eps=0.00001)
{
  extern __shared__ float _share[];

  //printf("blockDim.x=%d gridDim.x=%d \n", blockDim.x, gridDim.x);
  // blockDim.x=512 gridDim.x=1

  int cols = in.dim(1);

  assert(blockIdx.x < in.dim(0));
  assert(blockIdx.y < in.dim(2));
  assert(blockIdx.z < in.dim(3));

  float* _sum = _share + blockDim.x;
  _sum[threadIdx.x] = 0.0f;
  for (int tid = 0; tid < cols; tid += blockDim.x) {
    int id = tid + threadIdx.x;
    if (id < cols) {
      _sum[threadIdx.x] += in(blockIdx.x, id, blockIdx.y, blockIdx.z);
    }
  }
  __syncthreads();
  int len = blockDim.x;
  while(len != 1) {
    __syncthreads();
    int skip = (len + 1) >> 1;
    if (threadIdx.x < (len >> 1)) {
      _sum[threadIdx.x] += _sum[threadIdx.x + skip];
    }
    len = (len + 1) >> 1;
  }
  __syncthreads();
  float mean = _sum[0] / cols;
  __syncthreads();

  float* _sqSum = _share + blockDim.x;

  _sqSum[threadIdx.x] = 0.0;
  for (int tid = 0; tid < cols; tid += blockDim.x) {
    int id = tid + threadIdx.x;
    if(id < cols) {
      float ex = in(blockIdx.x, id, blockIdx.y, blockIdx.z) - mean;
      out(blockIdx.x, id, blockIdx.y, blockIdx.z) = ex;
      _sqSum[threadIdx.x] += ex * ex;
    }
  }
  __syncthreads();
  len = blockDim.x;
  while(len != 1) {
    __syncthreads();
    int skip = (len + 1) >> 1;
    if(threadIdx.x < (len >> 1))
      _sqSum[threadIdx.x] += _sqSum[threadIdx.x + skip];
    len = (len + 1) >> 1;
  }
  __syncthreads();
  float sigma = sqrtf(eps + (_sqSum[0] / cols));
  __syncthreads();

  for (int tid = 0; tid < cols; tid += blockDim.x) {
    int id = tid + threadIdx.x;
    if(id < cols) {
      float &val = out(blockIdx.x, id, blockIdx.y, blockIdx.z);
      if (betaWrap.size()) {
        val = alphaWrap[id] * (val / sigma) + betaWrap[id];
      } else {
        val = alphaWrap[id] * (val / sigma);
      }
    }
  }

}

void Normalization(Matrix &out,
                  const Matrix &in,
                  const Matrix &alpha,
                  const Matrix *beta,
                  float eps)
{
  assert(in.dim(0) < MAX_BLOCKS);
  assert(in.dim(2) < MAX_BLOCKS);
  assert(in.dim(3) < MAX_BLOCKS);

  //out.Reshape(in.dim(0), in.dim(1), in.dim(2), in.dim(3));

  int numThreads = std::min((uint) in.dim(1), (uint) MAX_THREADS);
  dim3 numBlocks(in.dim(0), in.dim(2), in.dim(3));
  int shared = numThreads * sizeof(float) * 2;

  MatrixWrapper<float> outWrap(out);
  const MatrixWrapper<float> inWrap(in);
  const MatrixWrapper<float> alphaWrap(alpha);
  MatrixWrapper<float> *betaWrap = beta ? new MatrixWrapper<float>(*beta) : new MatrixWrapper<float>();

  gLNormalization<<<numBlocks, numThreads, shared, CudaStreamHandler::GetStream()>>>
    (outWrap, inWrap, alphaWrap, *betaWrap, eps);

  /*
  //std::cerr << "nBlocks=" << numBlocks << std::endl;
  std::cerr << "nThreads=" << numThreads << std::endl;
  std::cerr << "outWrap=" << outWrap.Debug() << std::endl;
  std::cerr << "inWrap=" << inWrap.Debug() << std::endl;
  std::cerr << "alphaWrap=" << alphaWrap.Debug() << std::endl;
  std::cerr << "betaWrap=" << betaWrap->Debug() << std::endl;
  std::cerr << std::endl;

  HANDLE_ERROR(cudaDeviceSynchronize());
  */
  delete betaWrap;
}

void Normalization(Matrix& out, const Matrix& in, const Matrix& alpha, const Matrix& beta,
                       float eps)
{
  Normalization(out, in, alpha, &beta, eps);
}

void Normalization(Matrix& out, const Matrix& in, const Matrix& alpha, float eps)
{
  Normalization(out, in, alpha, nullptr, eps);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define LOWEST_FLOAT -1111111111111
#define HIGHEST_FLOAT +999999999999

__global__
void gBeamSizeInit(VectorWrapper<uint> hypo2BeamSize,
                    VectorWrapper<uint> activeBatch2Hypo,
                    VectorWrapper<uint> hypo2Candidate,
                    VectorWrapper<uint> hypo2NextHypo,
                    VectorWrapper<char> isFirsts,
                    const VectorWrapper<unsigned> beamSizes)
{
  uint nextHypoInd = 0;
  uint candidateInd = 0;

  uint hypoInd = 0, activeBatchInd = 0;
  //printf("beamSizes.size()=%u \n", beamSizes.size());
  for (size_t batchInd = 0; batchInd < beamSizes.size(); ++batchInd) {
    uint beamSize = beamSizes[batchInd];
    /*
    printf("batchInd=%u ", batchInd);
    printf("beamSize=%u ", beamSize);
    printf("hypoInd=%u ", hypoInd);
    printf("activeBatchInd=%u \n", activeBatchInd);
    */
    bool isFirst = isFirsts[batchInd];
    if (beamSize) {
      if (isFirst) {
        //printf("hypoInd=%i hypo2BeamSize=%i \n", hypoInd, hypo2BeamSize.size());
        assert(hypoInd < hypo2BeamSize.size());
        assert(hypoInd < hypo2Candidate.size());
        hypo2BeamSize[hypoInd] = beamSize;
        hypo2Candidate[hypoInd] = candidateInd;
        hypo2NextHypo[hypoInd] = nextHypoInd;
        ++hypoInd;

        assert(activeBatchInd < activeBatch2Hypo.size());
        activeBatch2Hypo[activeBatchInd] = batchInd;
        ++activeBatchInd;

        candidateInd += beamSize;
      }
      else {
        for (size_t j = 0; j < beamSize; ++j) {
          assert(hypoInd < hypo2BeamSize.size());
          assert(hypoInd < hypo2Candidate.size());
          hypo2BeamSize[hypoInd] = beamSize;
          hypo2Candidate[hypoInd] = candidateInd;
          hypo2NextHypo[hypoInd] = nextHypoInd;
          ++hypoInd;

          candidateInd += beamSize;
        }

        assert(activeBatchInd < activeBatch2Hypo.size());
        activeBatch2Hypo[activeBatchInd] = nextHypoInd;
        ++activeBatchInd;
      }

      nextHypoInd += beamSize;
    }
  }

  //printf("hypoInd=%i \n", hypoInd);
  //printf("activeBatchInd=%i \n", activeBatchInd);
}

__device__
float GetMaxScore(const MatrixWrapper<NthOutBatch> &nBestMatrix)
{
  float ret = LOWEST_FLOAT;
  for (uint i = 0; i < nBestMatrix.dim(1); ++i) {
      const NthOutBatch &curr = nBestMatrix[i];
      if (curr.score > ret) {
        ret = curr.score;
    }
  }

  return ret;
}

__device__
void AddElement(float &minScore,
    uint &i,
    VectorWrapper<NthOutBatch> &row,
    bool forbidUNK,
    uint vocabInd,
    const NthOutBatch &ele)
{
  const float score = ele.score;

  if (forbidUNK && vocabInd == UNK_ID) {
    row[i].score = LOWEST_FLOAT;
    minScore = LOWEST_FLOAT;
  }
  else {
    row[i] = ele;

    if (score < minScore) {
      minScore = score;
    }

    ++i;
  }

}

__device__
void MergeElement(float &minScore,
                  VectorWrapper<NthOutBatch> &row,
                  uint arrSize,
                  const NthOutBatch &ele)
{
  assert(arrSize <= row.size());

  float newMinScore = HIGHEST_FLOAT;
  bool found = false;
  for (uint i = 0; i < arrSize; ++i) {
    NthOutBatch &currEle = row[i];
    if (!found && minScore == currEle.score) {
      currEle = ele;
      found = true;
    }

    // update min score
    if (currEle.score < newMinScore) {
      newMinScore = currEle.score;
    }
  }

  minScore = newMinScore;
}

__device__
void MergeElement(float &minScore,
                  VectorWrapper<NthOutBatch> &row,
                  uint arrSize,
                  const NthOutBatch &ele,
                  bool forbidUNK,
                  uint vocabInd)
{
  if (forbidUNK && vocabInd == UNK_ID) {
    // do nothing
  }
  else if (ele.score > minScore) {
    // replace element with min score
    MergeElement(minScore, row, arrSize, ele);

    /*
    printf("arrInd=%d ind=%d vocabId=%d \n",
          arrInd,
          _max[threadIdx.x].ind,
          _max[threadIdx.x].vocabId);
    */
  }
}

__device__
void NBestAndMax(VectorWrapper<NthOutBatch> &nBestCandidates,
              float &topScore,
              const MatrixWrapper<float> &in,
              const MatrixWrapper<float> &b4,
              uint hypoInd,
              uint maxBeamSize,
              bool forbidUNK,
              const VectorWrapper<uint> &hypo2BeamSize,
              const VectorWrapper<uint> &hypo2Candidate)
{
  extern __shared__ char _sharePtr[];

  // placeholder for shared mem in subsequent function SumAndLogSoftMax
  //MatrixWrapper<float> maxMatrix((float*)_sharePtr, blockDim.x, 1, 1, 1);

  void *ptrOffset = _sharePtr + sizeof(float) * blockDim.x;
  MatrixWrapper<NthOutBatch> nBestMatrix((NthOutBatch*)ptrOffset, blockDim.x, maxBeamSize, 1, 1);
  VectorWrapper<NthOutBatch> row = nBestMatrix.Row(threadIdx.x);

  uint vocabSize = in.dim(1);

  assert(hypoInd < hypo2BeamSize.size());
  uint beamSize = hypo2BeamSize[hypoInd];

  float minScore = HIGHEST_FLOAT;

  // init
  uint vocabInd = threadIdx.x;
  uint i = 0;
  while (vocabInd < vocabSize && i < beamSize) {
    const float score = in(hypoInd, vocabInd, 0, 0) + b4(0, vocabInd, 0, 0);

    uint arrInd = hypoInd * vocabSize + vocabInd;
    NthOutBatch ele(arrInd, score, hypoInd, vocabInd);

    AddElement(minScore, i, row, forbidUNK, vocabInd, ele);

    vocabInd += blockDim.x;
  }

  // MAIN LOOP
  while (vocabInd < vocabSize) {
    const float score = in(hypoInd, vocabInd, 0, 0) + b4(0, vocabInd, 0, 0);
    uint arrInd = hypoInd * vocabSize + vocabInd;
    NthOutBatch ele(arrInd, score, hypoInd, vocabInd);

    MergeElement(minScore, row, beamSize, ele, forbidUNK, vocabInd);

    vocabInd += blockDim.x;
  } // while (vocabInd < vocabSize) {

  // merge nbest from different threads
  int len = blockDim.x;
  while (len != 1) {
    __syncthreads();
    int skip = (len + 1) >> 1;
    if (threadIdx.x < (len >> 1)) {
      for (uint i = 0; i < beamSize; ++i) {
        const NthOutBatch &ele = nBestMatrix(threadIdx.x + skip, i, 0, 0);
        if (ele.score > minScore) {
          MergeElement(minScore, row, beamSize, ele);
        }
      }
    }
    len = (len + 1) >> 1;

  }

  if (threadIdx.x == 0) {
    __syncthreads();

    // copy to output array
    assert(hypoInd < hypo2Candidate.size());
    uint candidateInd = hypo2Candidate[hypoInd];
    for (uint i = 0; i < beamSize; ++i) {
      const NthOutBatch &curr = nBestMatrix(0, i, 0, 0);

      //printf("candidateInd=%u nBestCandidates=%u \n", candidateInd, nBestCandidates.size());
      assert(candidateInd + i < nBestCandidates.size());
      nBestCandidates[candidateInd + i] = curr;
    }
  }

  __syncthreads();
  topScore = GetMaxScore(nBestMatrix);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
__device__
void SumAndLogSoftMax(VectorWrapper<NthOutBatch> &nBestCandidates,
                            const MatrixWrapper<float> &in,
                            const MatrixWrapper<float> &b4,
                            uint hypoInd,
                            uint maxBeamSize,
                            float topScore,
                            const VectorWrapper<uint> &hypo2BeamSize,
                            const VectorWrapper<uint> &hypo2Candidate)
{
  extern __shared__ float _share[];
  VectorWrapper<float> _sum(_share, blockDim.x);

  size_t vocabSize = in.dim(1);

  // calc sum
  _sum[threadIdx.x] = 0.0f;
  for (int id = threadIdx.x; id < vocabSize; id += blockDim.x) {
    //row[id] = exp(row[id] - max);
    float val = in(hypoInd, id, 0, 0) + b4(0, id, 0, 0);
    val = __expf(val - topScore);
    _sum[threadIdx.x] += val;
  }

  int len = blockDim.x;
  while (len != 1) {
    __syncthreads();

    int skip = (len + 1) >> 1;
    if (threadIdx.x < (len >> 1)) {
      _sum[threadIdx.x] += _sum[threadIdx.x + skip];
    }
    len = (len + 1) >> 1;
  }

  // apply partition and log to top
  if (threadIdx.x == 0) {
    __syncthreads();
    //printf("val=%f %f \n", in(rowIdx, ele.vocabId, 0, 0), val);

    // nbest
    uint beamSize = hypo2BeamSize[hypoInd];
    uint startPos = hypo2Candidate[hypoInd];
    for (uint i = 0; i < beamSize; ++i) {
      //__syncthreads();
      NthOutBatch &ele = nBestCandidates[startPos + i];

      float &val = ele.score;
      val = __expf(val - topScore);
      val = __logf(val /_sum[0]);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
__global__ void gLogSoftMax(VectorWrapper<NthOutBatch> nBestCandidates,
                        const MatrixWrapper<float> in,
                        const MatrixWrapper<float> b4,
                        uint maxBeamSize,
                        bool forbidUNK,
                        const VectorWrapper<uint> hypo2BeamSize,
                        const VectorWrapper<uint> hypo2Candidate)
{
  uint hypos = in.dim(0);
  uint vocabSize = in.dim(1);

  uint hypoInd =  blockIdx.x; // index of previous hypo
  while (hypoInd < hypos) {
    float topScore;

    NBestAndMax(nBestCandidates,
            topScore,
            in,
            b4,
            hypoInd,
            maxBeamSize,
            forbidUNK,
            hypo2BeamSize,
            hypo2Candidate);

    //__syncthreads();

    SumAndLogSoftMax(nBestCandidates,
                in,
                b4,
                hypoInd,
                maxBeamSize,
                topScore,
                hypo2BeamSize,
                hypo2Candidate);


    __syncthreads();
    hypoInd += gridDim.x;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
__global__ void gNBestPerBatch(VectorWrapper<NthOutBatch> nBest,
                        VectorWrapper<NthOutBatch> nBestCandidates,
                        const MatrixWrapper<float> in,
                        const VectorWrapper<float> costs,
                        uint maxBeamSize,
                        bool forbidUNK,
                        VectorWrapper<char> isFirsts,
                        const VectorWrapper<uint> hypo2BeamSize,
                        const VectorWrapper<uint> activeBatch2Hypo,
                        const VectorWrapper<uint> hypo2Candidate,
                        const VectorWrapper<uint> hypo2NextHypo)
{
  //uint rows = in.dim(0);
  uint activeBatchSize = activeBatch2Hypo.size();

  uint batchInd =  blockIdx.x;
  while (batchInd < activeBatchSize) {
    assert(batchInd < activeBatch2Hypo.size());
    assert(batchInd < hypo2BeamSize.size());
    assert(batchInd < nBest.size());

    uint hypoInd = activeBatch2Hypo[batchInd];
    uint beamSize = hypo2BeamSize[hypoInd];
    assert(beamSize);

    uint nextHypoInd = hypo2NextHypo[hypoInd];
    bool isFirst = isFirsts[batchInd];

    // candiate from 1st hypo
    float minScore = HIGHEST_FLOAT;
    assert(hypoInd < hypo2Candidate.size());
    uint candidateInd = hypo2Candidate[hypoInd];
    for (uint i = 0; i < beamSize; ++i) {
      //printf("prevHypoInd=%, candidateInd=%d \n", prevHypoInd, candidateInd);
      assert(hypoInd < costs.size());
      float prevCost = costs[hypoInd];

      assert((nextHypoInd + i) < nBest.size());
      assert(candidateInd + i < nBestCandidates.size());
      nBest[nextHypoInd + i] = nBestCandidates[candidateInd + i];
      //printf("HH1 nextHypoInd=%i candidateInd=%i i=%i hypoInd=%i \n", nextHypoInd, candidateInd, i, nBest[nextHypoInd + i].hypoInd);

      float &score = nBest[nextHypoInd + i].score;
      score += prevCost;

      if (score < minScore) {
        minScore = score;
      }
    }

    // candidates from other previous hypos
    if (!isFirst) {
      for (uint hypoOffset = 1; hypoOffset < beamSize; ++hypoOffset) {
        //printf("hypoInd=%d \n", (hypoInd + hypoOffset));

        //printf("prevHypoInd=%, candidateInd=%d \n", prevHypoInd, candidateInd);
        assert((hypoInd + hypoOffset) < costs.size());
        float prevCost = costs[hypoInd + hypoOffset];

        assert((hypoInd + hypoOffset) < hypo2Candidate.size());
        uint candidateInd = hypo2Candidate[hypoInd + hypoOffset];

        for (uint candidateOffset = 0; candidateOffset < beamSize; ++candidateOffset) {
          assert((candidateInd + candidateOffset) < nBestCandidates.size());
          NthOutBatch &candidate = nBestCandidates[candidateInd + candidateOffset];
          candidate.score += prevCost;

          assert(nextHypoInd < nBest.size());
          VectorWrapper<NthOutBatch> offset = nBest.Offset(nextHypoInd);

          if (candidate.score > minScore) {
            MergeElement(minScore, offset, beamSize, candidate);
          }
        }
      }
    }

    batchInd += gridDim.x;
  }
  //printf("\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void LogSoftmaxAndNBest(mblas::Vector<NthOutBatch> &nBest,
                const Matrix& in,
                const Matrix& b4,
                const mblas::Vector<float> &costs,
                const Histories& histories,
                bool forbidUNK,
                uint maxBeamSize)
{
  //BEGIN_TIMER("LogSoftmax excl kernels");
  //HANDLE_ERROR( cudaStreamSynchronize(mblas::CudaStreamHandler::GetStream()));
  //cerr << "LogSoftmaxAndNBest0" << endl;

  //cerr << "in=" << in.Debug(0) << endl;
  //cerr << "histories=" << histories.Debug(1) << endl;

  std::vector<char> isFirsts = histories.IsFirsts();

  // create beam size vectors on GPU but exclude empty beams
  uint candidateInd = histories.NumCandidates();
  uint activeBatchSize = histories.NumActive();
  uint numHypos = in.dim(0);
  uint numNextHypos = histories.GetTotalBeamSize();

  mblas::Vector<char> d_isFirsts(isFirsts);
  mblas::Vector<unsigned> d_beamSizes(histories.GetBeamSizes());
  mblas::Vector<uint> hypo2BeamSize(numHypos);
  mblas::Vector<uint> hypo2Candidate(numHypos);
  mblas::Vector<uint> hypo2NextHypo(numHypos);
  mblas::Vector<uint> activeBatch2Hypo(activeBatchSize);
  mblas::Vector<NthOutBatch> nBestCandidates(candidateInd);
  //PAUSE_TIMER("LogSoftmax excl kernels");

  //cerr << "LogSoftmaxAndNBest1" << endl;

  //BEGIN_TIMER("gBeamSizeInit");
  gBeamSizeInit<<<1, 1, 0, CudaStreamHandler::GetStream()>>>
    (hypo2BeamSize,
    activeBatch2Hypo,
    hypo2Candidate,
    hypo2NextHypo,
    d_isFirsts,
    d_beamSizes
    );
  //PAUSE_TIMER("gBeamSizeInit");
  HANDLE_ERROR( cudaStreamSynchronize(mblas::CudaStreamHandler::GetStream()));
  /*
  cerr << "numHypos=" << numHypos << endl;
  cerr << "numNextHypos=" << numNextHypos << endl;
  cerr << "isFirsts=" << Debug(isFirsts, 2) << endl;
  cerr << "in=" << in.Debug(0) << endl;
  cerr << "activeBatchSize=" << activeBatchSize << endl;
  cerr << "candidateInd=" << candidateInd << endl;
  cerr << "hypo2BeamSize=" << hypo2BeamSize.Debug(2) << endl;
  cerr << "hypo2Candidate=" << hypo2Candidate.Debug(2) << endl;
  cerr << "nBest=" << nBest.Debug(2) << endl;
  cerr << "nBestCandidates=" << nBestCandidates.Debug(2) << endl;
  cerr << "histories=" << histories.Debug(2) << endl;
  cerr << "activeBatch2Hypo=" << activeBatch2Hypo.Debug(2) << endl;
  cerr << "hypo2NextHypo=" << hypo2NextHypo.Debug(2) << endl;
  cerr << endl;
  */

  int blocks = std::min(MAX_BLOCKS, (int)numHypos);
  int threads = std::min(MAX_THREADS, (int)in.dim(1));
  int shared = sizeof(NthOutBatch) * threads * maxBeamSize
             + sizeof(float) * threads;
  //HANDLE_ERROR( cudaStreamSynchronize(mblas::CudaStreamHandler::GetStream()));
  //cerr << "LogSoftmaxAndNBest2" << endl;
  //BEGIN_TIMER("gLogSoftMax");
  gLogSoftMax<<<blocks, threads, shared, CudaStreamHandler::GetStream()>>>
    (nBestCandidates,
     in,
     b4,
     maxBeamSize,
     forbidUNK,
     hypo2BeamSize,
     hypo2Candidate);
  //PAUSE_TIMER("gLogSoftMax");
  //HANDLE_ERROR( cudaStreamSynchronize(mblas::CudaStreamHandler::GetStream()));
  //cerr << "LogSoftmaxAndNBest3" << endl;
  //cerr << "nBestCandidates=" << nBestCandidates.Debug(2) << endl;

  blocks = std::min(MAX_BLOCKS, (int)activeBatchSize);

  //BEGIN_TIMER("gNBestPerBatch");
  gNBestPerBatch<<<blocks, 1, 0, CudaStreamHandler::GetStream()>>>
    (nBest,
     nBestCandidates,
     in,
     costs,
     maxBeamSize,
     forbidUNK,
     d_isFirsts,
     hypo2BeamSize,
     activeBatch2Hypo,
     hypo2Candidate,
     hypo2NextHypo);
  //PAUSE_TIMER("gNBestPerBatch");
  //HANDLE_ERROR( cudaStreamSynchronize(mblas::CudaStreamHandler::GetStream()));
  //cerr << "LogSoftmaxAndNBest4" << endl;

  //HANDLE_ERROR( cudaStreamSynchronize(mblas::CudaStreamHandler::GetStream()));
  //cerr << "step3" << endl;
  //cerr << "3costs=" << Debug(costs, 0) << endl;
}

__global__
void gUpdateSentenceLengths(const VectorWrapper<uint> newSentenceLengths,
                            const VectorWrapper<uint> newBatchIds,
                            VectorWrapper<uint> sentenceLengths)
{
  uint id =  threadIdx.x; // index of previous hypo
  while (id < newSentenceLengths.size()) {
    uint sentenceLength = newSentenceLengths[id];
    uint batchId = newBatchIds[id];

    assert(batchId < sentenceLengths.size());
    sentenceLengths[batchId] = sentenceLength;

    id += blockDim.x;
  }
}

void UpdateSentenceLengths(const mblas::Vector<uint> &newSentenceLengths,
                          const mblas::Vector<uint> &newBatchIds,
                          mblas::Vector<uint> &sentenceLengths)
{
  assert(newSentenceLengths.size() == newBatchIds.size());
  assert(newSentenceLengths.size() <= sentenceLengths.size());

  int blocks = 1;
  int threads = std::min(MAX_THREADS, (int) newSentenceLengths.size());

  gUpdateSentenceLengths<<<blocks, threads, 0, CudaStreamHandler::GetStream()>>>(newSentenceLengths, newBatchIds, sentenceLengths);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__global__
void gAddNewData(mblas::MatrixWrapper<float> dest,
                const mblas::MatrixWrapper<float> source,
                size_t batchId,
                size_t newSentenceOffset,
                size_t size)
{
  int id = threadIdx.x + blockIdx.x * blockDim.x;
  if (id < size) {
    size_t dim0 = id / source.dim(1);
    size_t dim1 = id % source.dim(1);

    dest(dim0, dim1, 0, batchId) = source(dim0, dim1, 0, newSentenceOffset);
  }
}

void AddNewData(mblas::Matrix &sourceContext,
                const vector<uint> &newBatchIds,
                const std::vector<BufferOutput> &newSentences)
{
  BEGIN_TIMER("AddNewData");
  //cerr << "sourceContext=" << sourceContext.Debug(0) << endl;

  for (size_t i = 0; i < newSentences.size(); ++i) {
    const BufferOutput &eleSent = newSentences[i];
    const EncOutPtr &encOut = eleSent.GetEncOut();
    const mblas::Matrix &newSourceContext = encOut->Get<EncOutGPU>().GetSourceContext();
    //cerr << "newSourceContext=" << newSourceContext.Debug(0) << endl;

    size_t batchId = newBatchIds[i];
    size_t newSentenceOffset = eleSent.GetSentenceOffset();

    assert(batchId < sourceContext.dim(3));
    assert(newSentenceOffset < newSourceContext.dim(3));
    assert(sourceContext.dim(0) >= newSourceContext.dim(0));
    assert(sourceContext.dim(1) == newSourceContext.dim(1));
    assert(sourceContext.dim(2) == newSourceContext.dim(2) == 1);

    size_t size = newSourceContext.dim(0) * newSourceContext.dim(1);
    uint threads = std::min((uint) MAX_THREADS, (uint)size);
    uint blocks  = size / threads + ((size % threads == 0) ?  0 : 1);

    mblas::MatrixWrapper<float> dest(sourceContext);
    const mblas::MatrixWrapper<float> source(newSourceContext);

    gAddNewData<<<blocks, threads, 0, CudaStreamHandler::GetStream()>>>(dest, source, batchId, newSentenceOffset, size);
  }

  PAUSE_TIMER("AddNewData");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestMemCpy()
{
  using namespace std;

  cerr << "Starting" << endl;

  size_t NUM = 10;
  vector<float> h_vec1(NUM);
  for (size_t i = 0; i < NUM; ++i) {
    h_vec1[i] = i * 3;
  }

  TestMemCpy(NUM, h_vec1.data());

  cerr << "Finished" << endl;
}

}  // namespace mblas
}  // namespace GPU
}  // namespace amunmt
