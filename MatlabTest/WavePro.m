clc
clear all


rawData=csvread("wave.csv");
rawData=rawData-15;
N=length(rawData);
n=[0:1:N-1]; 

filtData=smoothdata(rawData,1,"movmean",10);
peekInd=AMPD(filtData,1);
peekInd=[78,peekInd];

minuspoint=zeros(1,6);
j=1;
for i=2:N
    if filtData(i)<0 && filtData(i-1)>=0
        if filtData(i-1)>filtData(i)&&filtData(i+1)<filtData(i)
            minuspoint(j)=i;
            j=j+1;
        end
    end
end

% figure
% % subplot(413);
% % plot(n,base_line(1:N));title("base");hold on;
% % plot(n,zeros(1,N));
% subplot(211);
% plot(filtData(1:N));title("");hold on;
% plot(n,zeros(1,N));hold on;
% plot(n,zeros(1,N));
% subplot(414);
% plot(n,nobase(1:N));hold on;title("去基线处理");
% plot(n,zeros(1,N));hold on;
% scatter(peekInd,nobase(peekInd),'r.');%ylim([-50 1000])
% subplot(212);
plot(filtData(1:N));title("平滑滤波");hold on;
plot(n,zeros(1,N));hold on;
scatter(peekInd,filtData(peekInd),'r.');%ylim([-50 1000])
scatter(minuspoint,filtData(minuspoint),'b.');%ylim([-50 1000])

ylim([-250,250]);
% plot(n,zeros(1,N));
