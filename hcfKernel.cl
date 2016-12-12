__kernel void hcfKernel(__global const float2* x1,__global const float2* x2, __global float2* y, __global float* sumR1,__global float* sumI1,__global float* sumR2, int n, int PHI)
{
int gl = get_global_size(0);
int tx = get_global_id(0);
int kot=tx+PHI-1;
int kot2=tx;

__private float hR1;
__private float hI1;

__private float sR1=sumR1[0];
__private float sI1=sumI1[0];
__private float sR2=sumR2[0];

//---------------------------------------------konczy h-----------------------------------------

hR1=sR1/sR2;
hI1=sI1/sR2;
//---------------------------------------------odejmuje iksy------------------------------------

while(kot<n){//dlatego mogly sie nie zgadzac wyniki -czasem za duzo liczyl
		y[kot].x -=  (hR1 * x2[kot2].x - hI1 * x2[kot2].y);
		y[kot].y -=  (hI1 * x2[kot2].x + hR1 * x2[kot2].y);
		kot+=gl;
		kot2+=gl;
	}

//---------------------------------------------oblicza k----------------------------------------

sR1=0;
sI1=0;
sR2=0;

kot=tx+PHI;
kot2=tx;

while(kot<n){//conj(x2)*x1/[x2*conj(x2) + x1*conj(x1)]
		sR1  += (x1[kot].x * x2[kot2].x + x1[kot].y * x2[kot2].y);
		sI1  += (x1[kot].y * x2[kot2].x - x1[kot].x * x2[kot2].y);
		sR2  += (x2[kot2].x * x2[kot2].x + x2[kot2].y * x2[kot2].y)+(x1[kot].x * x1[kot].x + x1[kot].y * x1[kot].y);
		kot+=gl;
		kot2+=gl;
	}

//mem_fence(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
sumR1[tx] = 2*sR1;
sumI1[tx] = 2*sI1;
sumR2[tx] = sR2;

}