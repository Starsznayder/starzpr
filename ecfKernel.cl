__kernel void ecfKernel(__global float2* x1,__global float2* x2, __global const float2* y, __global float* sumR1,__global float* sumI1,__global float* sumR2, int n, int PHI)
{
int gl = get_global_size(0);
int tx = get_global_id(0);
int kot=tx+PHI;
int kot2=tx;

__private float kR1;
__private float kI1;


__private float2 tmpx1;
__private float2 tmpx2;

__private float sR1=sumR1[0];
__private float sI1=sumI1[0];
__private float sR2=sumR2[0];

//---------------------------------------------oblicza k----------------------------------------

kR1=sR1/sR2;
kI1=sI1/sR2;
//----------------------------------------odejmuje k*x od x'ow----------------------------------


while(kot<n){//conj(k) przy odejmowaniu x1 od x2
		tmpx2=x2[kot2];
		tmpx1=x1[kot];
		x1[kot].x=tmpx1.x - (kR1 * tmpx2.x - kI1 * tmpx2.y);
		x1[kot].y=tmpx1.y - (kR1 * tmpx2.y + kI1 * tmpx2.x);
		x2[kot2].x =tmpx2.x - (kR1 * tmpx1.x + kI1 * tmpx1.y);
		x2[kot2].y =tmpx2.y - (kR1 * tmpx1.y - kI1 * tmpx1.x);
		kot+=gl;
		kot2+=gl;
	}

//----------------------------------------liczy h----------------------------------

sR1=0;
sI1=0;
sR2=0;

kot = tx + PHI; 
kot2 = tx; 
while(kot<n){
		sR1 += (y[kot].x * x2[kot2].x + y[kot].y * x2[kot2].y);
		sI1 += (y[kot].y * x2[kot2].x - y[kot].x * x2[kot2].y);
		sR2 += (x2[kot2].x * x2[kot2].x + x2[kot2].y * x2[kot2].y);
		kot+=gl;
		kot2+=gl;
	}

//mem_fence(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
sumR1[tx] = sR1;
sumI1[tx] = sI1;
sumR2[tx] = sR2;

}