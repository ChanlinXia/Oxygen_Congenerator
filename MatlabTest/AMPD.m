% function indexMax=AMPD(data)
% 
% N=length(data);
% L=ceil(N/2)-1;
% M=zeros(L,N);
% for k=1:L
%     for i=1:N
%         if i<k+1||i>N-k
%             M(k,i)=1+rand;
%         elseif data(i)<data(i-k)&&data(i)<=data(i+k)
%             M(k,i)=0;
%         else
%             M(k,i)=1+rand;
%         end
%     end
% end
% rowsum=sum(M,2);
% [~,MaxWinLen]=min(rowsum)
% M=M(1:MaxWinLen,:);
% sigma=std(M,0,1);
% indexMax=find(~sigma);
% end

% function indexMax=AMPD(data,startPos)
% 
% i=0;k=0;j=0;
% arrSumTemp=zeros(1,ceil(length(data)/2));
% peeksPosTemp=zeros(1,ceil(length(data)));
% maxWinLen=0;sumTemp=0;indexMax=[];
% dataLen=length(data);
% 
% for k=1:ceil(length(data)/2)+1
%     sumTemp=0;
%     for i=k+1:length(data)-k
%         if data(i)>data(i-k) && data(i)>=data(i+k)
%              sumTemp=sumTemp+1;
%         end
%     end
%     arrSumTemp(k)=sumTemp;
% end
% 
% [~,maxWinLen]=max(arrSumTemp)
% 
% for k=1:maxWinLen+2
%     for i=k+1:dataLen-k
%         if data(i)>data(i-k) && data(i)>=data(i+k)
%              peeksPosTemp(i)=peeksPosTemp(i)+1;
%         end
%     end
% end
% 
% j=1;
% for i=startPos:length(data)
%     if peeksPosTemp(i) == maxWinLen
%         indexMax(j)=i;
%         j=j+1;
%     end
% end
% 
% for i=1:startPos
%     if peeksPosTemp(i) == maxWinLen
%         indexMax(j)=i;
%         j=j+1;
%     end
% end

% function indexMax=AMPD(data,startPos)
% 
% i=0;k=0;j=0;
% arrSumTemp=zeros(1,ceil(length(data)/2));
% peeksPosTemp=zeros(1,ceil(length(data)));
% maxWinLen=0;sumTemp=0;indexMax=[];
% dataLen=length(data);
%

function indexMax=AMPD(data,startPos)

i=0;k=0;j=0;
arrSumTemp=zeros(1,ceil(length(data)/2));
peeksPosTemp=zeros(1,ceil(length(data)));
maxWinLen=0;sumTemp=0;indexMax=[];
dataLen=length(data);
for k=1:ceil(length(data)/2)+1
    sumTemp=0;
    for i=k+1:length(data)-k
        if data(i)<data(i-k) && data(i)<=data(i+k)
             sumTemp=sumTemp+1;
        end
    end
    arrSumTemp(k)=sumTemp;
end

[~,maxWinLen]=max(arrSumTemp)

for k=1:maxWinLen+2
    for i=k+1:dataLen-k
        if data(i)<data(i-k) && data(i)<=data(i+k)
             peeksPosTemp(i)=peeksPosTemp(i)+1;
        end
    end
end

j=1;
for i=startPos:length(data)
    if peeksPosTemp(i) == maxWinLen
        indexMax(j)=i;
        j=j+1;
    end
end

for i=1:startPos
    if peeksPosTemp(i) == maxWinLen
        indexMax(j)=i;
        j=j+1;
    end
end



% function varargout = AMPD(Signal)
% %AMPD Automatic Multiscale-based Peak Detection
% %   INPUTS:
% %       Signal [*] - This is a real vector containing the data from the
% %                    maxima will be found.
% % 
% %   OUTPUTS:
% %       index     - The indicies of the identified peaks in "Signal".
% % 
% %   USAGE:
% %       [index] = ampd(Signal)
% %                 Returns the indicies of local maxima detected in 
% %                 'Signal'.
% % 
% %   REFERENCES:
% %       [ 1 ] Scholkmann F., Boss J. and Wolf M. "An Efficient Algorithm 
% %             for Automatic Peak Detection in Noisy Periodic and 
% %             Quasi-Periodic Signals." Algorithms 2012, 5, 588-603;
% %             doi:10.3390/a5040588.
% % 
% 	narginchk(1,1);
%     nargoutchk(0,1);
% 
%     if ~isvector(Signal) || isscalar(Signal)
%         error('Input argument ''Signal'' must be a vector')
% 
%     elseif ~isfloat(Signal)
%         error('Class of input argument ''Signal'' must be ''double''')
% 
%     elseif ~isrow(Signal)
%         Signal = Signal';
% 
%     end
% 
%     time = 1:numel(Signal);
% 
%     [fitPolynomial,fitError] = polyfit(time,Signal,1);
%     [fitSignal,~] = polyval(fitPolynomial,time,fitError);
% 
%     dtrSignal = Signal - fitSignal;
% 
%     N = length(dtrSignal);
%     L = ceil(N/2.0)-1;
% 
%     RNG = RandStream('mt19937ar');
%     LSM = 1 + rand(RNG,L,N);
% 
%     for k=1:L
%         for i=k+2:N-k+1
%             if(Signal(i-1)>Signal(i-k-1) && Signal(i-1)>Signal(i+k-1))
%                 LSM(k,i) = 0;
%             end
%         end
%     end
% 
% 	G = sum(LSM,2);
%     [~,l] = min(G);
% 
%     LSM = LSM(1:l,:);
% 
%     S = std(LSM);
% 
%     varargout{1} = find(S==0)-1;
% end