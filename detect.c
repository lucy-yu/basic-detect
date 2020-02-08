#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
typedef int  uint16_t;
short max_cal(short *buf,uint16_t len);
float voice_cal_frame_acf(short *buf, uint16_t len);
void fredeal(short *fifo_data, uint16_t len);
float GetMedianNum(float *period, int iFilterLen);
float medianfilter(float *period, int len,int filterlen);
int main()
{
    //心跳
    short fifo_datayuan1[]={
};//音频数组
    int aflen=sizeof(fifo_datayuan)/sizeof(short);
    short fifo_data[aflen/2+1];
    for(int i=0,j=0;i<aflen;i+=2)
    {
        fifo_data[j]=fifo_datayuan[i];
        j++;
    }
    //chnnel num==2 change to chnnel 1//
    int len=sizeof(fifo_data)/sizeof(short);
    printf("%d\n",len);//数组的长度
    fredeal(fifo_data,len);
	//bandpass_filter();//滤波
	//分帧 inc；帧移 overlap:重叠 wlen:每帧长度 fn:帧数
	int inc=200;
	int wlen=240;
	short buf[wlen];
	float sum=0;
	int startindex;
	int fn=(len-wlen)/(wlen-inc)+1;
    printf("fn is %d",fn);
	float period[fn];
	for(int i=1;i<=fn;i++)
	{
		startindex=(i-1)*inc+1;
		for(int j=0;j<wlen;j++)
		{
			buf[j]=fifo_data[j+(i-1)*wlen-startindex+1];
			//printf("zhen is %d\n",j+(i-1)*wlen-startindex+1);
		}
		period[i]=voice_cal_frame_acf(buf,wlen);
	}
	float meanfre=medianfilter(period,fn,5);//中值滤波
    printf("meanfre is %f",meanfre);
    return 0;
}

short max_cal(short *buf,uint16_t len)
{
	short max=buf[0];
	for(int m=0;m<len;m++)
		{
			if(buf[m]>max)
			{
				max=buf[m];
			}
	}
	return max;
}
float voice_cal_frame_acf(short *buf, uint16_t len)
{
	float acf[len-1];
	int samplerate=800;
	short mx=max_cal(buf,len);
	float cl=mx*0.6;
	for(int j=0;j<len;j++)
	{
		if (buf[j] >cl){buf[j]=buf[j]-cl;}
		else if(buf[j] <=(-cl)){buf[j]=buf[j]+cl;}//中心消波
		else{buf[j]=0;}
	}
	short mx2=max_cal(buf,len);
    float inrelate[len];
	for(int m=0;m<len;m++)
	{
		inrelate[m]=(float)buf[m]/(float)mx2;//归一化
	}

	for(int k=0;k<len;k++)
	{
        float sum=0.0;
		for(int i=0;i<len-k;i++)
		{
			sum+=inrelate[i]*inrelate[i+k];
		}
		acf[k]=sum;
	}//由于对称性算一半k
	float maxaf=acf[0];
	float acfshot[len-1];
	for(int h=0;h<len;h++)
	{
       // if(acfshot[h]==1)
		//printf("maxok  %f  --- %d\n",acfshot[h],h);
		acfshot[h]=acf[h]/maxaf;
	}
	int index=0;
	int pmax=floor(samplerate/20);
	int pmin=floor(samplerate/400);
	float max=-999;//任意负数
	for(int n=pmin;n<pmax;n++)//取整
	{
		if(acfshot[n]>max)
			{
				max=acfshot[n];
				index=n;
			}
        //else{printf("error");}
	}
    //printf("T is ---%d,fre is%d\n",index,(short)samplerate/index);//基音周期,频点
	return (float)samplerate/(float)(index+pmin-1);
}
void fredeal(short *fifo_data, uint16_t len)
{
    short sum=0,mean=0;
    for(int i=0;i<len;i++)
    {
        sum+=fifo_data[i];
    }
    mean=sum/len;
    for(int i=0;i<len;i++)
    {
       fifo_data[i]=fifo_data[i] -  mean;
    }
}
float GetMedianNum(float *buf, int iFilterLen)
{
	int i,j;
	float pTemp;
	for (i=0; i<iFilterLen-1;i++)
	{
		for (j=0; j<iFilterLen-i-1; j++)
		{
			if (buf[j] > buf[j+1])
			{
				pTemp = buf[j];
				buf[j] = buf[j+1];
				buf[j+1] = pTemp;
			}
		}
	}
	if (iFilterLen%2!=0)
	{
		pTemp = buf[(iFilterLen+1)/2-1];
	}
	else
	{
		pTemp = (buf[iFilterLen/2-1] + buf[iFilterLen/2])/2;
	}
	return pTemp;
}
float medianfilter(float *period, int len,int filterlen)
{
    float media;
    float buf[filterlen];
    float sum=0.0;
    for(int i=0;i<len;i++)
    {
        for(int j=0;j<filterlen;j++)
        {
            buf[j]=period[j+i];
            //printf("buff is %f\n",buf[j]);
        }
       // printf("hello");
        media=GetMedianNum(buf,filterlen);
        //printf("media is %d\n",media);
        period[i+1]=media;
    }
    for(int i=0;i<len;i++)
    {
        sum+=period[i];
    }
    return sum/len;
}
