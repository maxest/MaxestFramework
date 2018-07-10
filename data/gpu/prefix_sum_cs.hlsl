// https://www.mimuw.edu.pl/~ps209291/kgkp/slides/scan.pdf


static const int ElementsCount = ELEMENTS_COUNT;
static const int LevelsCount = log2(ElementsCount);


#ifdef PREFIX_SUM
	RWStructuredBuffer<uint> outputBuffer_perBlockPrefixSums: register(u0);
	#ifdef WRITE_BLOCKS_SUMS
		RWStructuredBuffer<uint> outputBuffer_blocksSums: register(u1);
	#endif

	StructuredBuffer<uint> inputBuffer: register(t0);


	#if VARIANT == 1
		groupshared uint tempData[2 * ElementsCount];


		void PrefixSum(int threadID)
		{
			int array1Offset = 0;
			int array2Offset = ElementsCount;
			int stride = 1;

			[unroll]
			for (int i = 1; i <= LevelsCount; i++)
			{
				GroupMemoryBarrierWithGroupSync();

				if (threadID >= stride)
					tempData[array1Offset + threadID] = tempData[array2Offset + threadID] + tempData[array2Offset + threadID - stride];
				else
					tempData[array1Offset + threadID] = tempData[array2Offset + threadID];

				array1Offset = ElementsCount - array1Offset;
				array2Offset = ElementsCount - array2Offset;
				stride *= 2;
			}

			GroupMemoryBarrierWithGroupSync();
		}


		[numthreads(ElementsCount, 1, 1)]
		void main(uint3 gID: SV_GroupID, uint3 gtID: SV_GroupThreadID)
		{
			int blockID = gID.x;
			int threadID = gtID.x;

			int ai = threadID;

			uint value = inputBuffer[ElementsCount*blockID + ai];
			tempData[ai] = value;
			tempData[ElementsCount + ai] = value;

			PrefixSum(threadID);

			int tempDataOffset = (LevelsCount % 2 == 0 ? ElementsCount : 0);
			outputBuffer_perBlockPrefixSums[ElementsCount*blockID + ai] = tempData[tempDataOffset + ai];

		#ifdef WRITE_BLOCKS_SUMS
			if (threadID == ElementsCount - 1)
				outputBuffer_blocksSums[blockID] = outputBuffer_perBlockPrefixSums[ElementsCount*blockID + ElementsCount - 1];
		#endif
		}
	#elif VARIANT == 2
		groupshared uint tempData[ElementsCount];	


		void PrefixSum(int threadID)
		{
			int stride = 1;
			int activeThreadsCount = ElementsCount / 2;

			for (int i = 0; i < LevelsCount; i++)
			{
				GroupMemoryBarrierWithGroupSync();

				if (threadID < activeThreadsCount)
				{
					int ai = stride*(2*threadID + 1) - 1;
					int bi = stride*(2*threadID + 2) - 1;

					tempData[bi] += tempData[ai];
				}

				stride *= 2;
				activeThreadsCount >>= 1;
			}

			if (threadID == 0)
				tempData[ElementsCount - 1] = 0;

			stride = ElementsCount / 2;
			activeThreadsCount = 1;

			for (int i = 0; i < LevelsCount; i++)
			{
				GroupMemoryBarrierWithGroupSync();

				if (threadID < activeThreadsCount)
				{
					int ai = stride*(2*threadID + 1) - 1;
					int bi = stride*(2*threadID + 2) - 1;

					uint t = tempData[ai];
					tempData[ai] = tempData[bi];
					tempData[bi] += t;
				}

				stride >>= 1;
				activeThreadsCount *= 2;
			}
			
			GroupMemoryBarrierWithGroupSync();
		}


		[numthreads(ElementsCount/2, 1, 1)]
		void main(uint3 gID: SV_GroupID, uint3 gtID: SV_GroupThreadID)
		{
			int blockID = gID.x;
			int threadID = gtID.x;

			int ai = threadID;
			int bi = ElementsCount/2 + threadID;

			uint value1 = inputBuffer[ElementsCount*blockID + ai];
			uint value2 = inputBuffer[ElementsCount*blockID + bi];
			tempData[ai] = value1;
			tempData[bi] = value2;

			PrefixSum(threadID);

			outputBuffer_perBlockPrefixSums[ElementsCount*blockID + ai] = value1 + tempData[ai];
			outputBuffer_perBlockPrefixSums[ElementsCount*blockID + bi] = value2 + tempData[bi];

		#ifdef WRITE_BLOCKS_SUMS
			if (threadID == ElementsCount/2 - 1)
				outputBuffer_blocksSums[blockID] = outputBuffer_perBlockPrefixSums[ElementsCount*blockID + ElementsCount - 1];
		#endif
		}
	#endif
#endif



#ifdef SUM
	RWStructuredBuffer<uint> outputBuffer: register(u0);

	StructuredBuffer<uint> inputBuffer_cumulativeBlocksSums: register(t0);


	[numthreads(ElementsCount/2, 1, 1)]
	void main(uint3 gID: SV_GroupID, uint3 gtID: SV_GroupThreadID)
	{
		int blockID = 1 + gID.x;
		int threadID = gtID.x;
		
		int ai = ElementsCount*blockID + threadID;
		int bi = ElementsCount*blockID + ElementsCount/2 + threadID;

		outputBuffer[ai] += inputBuffer_cumulativeBlocksSums[gID.x];
		outputBuffer[bi] += inputBuffer_cumulativeBlocksSums[gID.x];
	}
#endif
