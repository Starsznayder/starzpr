__kernel void ecKernel(__global const float2* x, __global float2* y, __global float* sumR1,__global float* sumI1,__global float* sumR2, int n, int PHI)
{
int i=0;
int gl = get_global_size(0);
int tx = get_global_id(0);


__private float sR1=0;
__private float sI1=0;
__private float sR2=0;
//mem_fence(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);


while(tx+PHI+i*(gl)<n){//dlatego mogly sie nie zgadzac wyniki -czasem za duzo liczyl
		sR1  += (y[tx+PHI+i*gl].x * x[tx+i*gl].x + y[tx+PHI+i*gl].y * x[tx+i*gl].y);
		sI1  += (y[tx+PHI+i*gl].y * x[tx+i*gl].x - y[tx+PHI+i*gl].x * x[tx+i*gl].y);
		sR2  += (x[tx+i*gl].x * x[tx+i*gl].x + x[tx+i*gl].y * x[tx+i*gl].y);
		i++;
	}
//mem_fence(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
sumR1[tx] = sR1;
sumI1[tx] = sI1;
sumR2[tx] = sR2;

//barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
}
