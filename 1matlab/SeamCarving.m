function [resizedImage] = ModifiedSeamCarving(ImgName, hor, ver)
InputImage = imread(ImgName);
GrayImage = rgb2gray(InputImage);
GradMean = CalcGradient(GrayImage);
ReducedInputImage = InputImage;
ReducedGrayImage = GrayImage;
ReducedGradMean = GradMean;
for i=1:1:min(ver, hor)
[ReducedInputImage,ReducedGradMean, ReducedGrayImage] = RemoveVer(ReducedGradMean, ReducedInputImage, ReducedGrayImage);
[ReducedInputImage,ReducedGradMean, ReducedGrayImage] = RemoveHor(ReducedGradMean, ReducedInputImage, ReducedGrayImage);
end
for j=1:1:(max(ver,hor)-min(ver,hor))
    if(ver>hor)
        [ReducedInputImage,ReducedGradMean, ReducedGrayImage] = RemoveVer(ReducedGradMean, ReducedInputImage, ReducedGrayImage);
    else
        [ReducedInputImage,ReducedGradMean, ReducedGrayImage] = RemoveHor(ReducedGradMean, ReducedInputImage, ReducedGrayImage);
    end
end
ReducedGrayImage = uint8(ReducedGrayImage);
ReducedInputImage = uint8(ReducedInputImage);
figure
imshow(InputImage);
figure
imshow(ReducedGrayImage);
figure
imshow(ReducedInputImage);
end

% calculating gradient for input image
function [GradMean] = CalcGradient(inputImage)
    sobelOpx = [ 1 0 -1 ; 2  0  -2; 1 0 -1];
    sobelOpy = [1 2 1; 0 0 0; -1 -2 -1];
    [rows, cols, dim] = size(inputImage);
    for i= 1:1:dim
        GradHor(:,:,i) = filter2(sobelOpx, inputImage(:,:,i));
        GradVer(:,:,i) = filter2(sobelOpy, inputImage(:,:,i));
        Grad(:,:,i) = abs(GradHor(:,:,i)) + abs(GradVer(:,:,i));
    end
    GradMean = sum(Grad, 3)/dim;
end

%Remove Vertial Seam
function [ReducedInputImage,ReducedGradMean, ReducedGrayImage] = RemoveVer(ReducedGradMean, ReducedInputImage, ReducedGrayImage)
EnergyMapVer = CalcEnergyMapForward(ReducedGradMean, ReducedGrayImage);
SeamVer = findSeam(EnergyMapVer);
ReducedInputImage = RemoveSeam(ReducedInputImage,SeamVer);
ReducedGrayImage = RemoveSeam(ReducedGrayImage,SeamVer);
ReducedGradMean = RemoveSeamGradient(ReducedGradMean, SeamVer);
end

% Remove Horizontal Seam. It uses same functions as vertical so we need to
% transpose images before use those functions.
function [ReducedInputImage,ReducedGradMean, ReducedGrayImage] = RemoveHor(ReducedGradMean, ReducedInputImage, ReducedGrayImage)
EnergyMapHor = CalcEnergyMapForward(ReducedGradMean', permute(ReducedGrayImage,[2,1,3]));
SeamHor = findSeam(EnergyMapHor);
ReducedInputImage = RemoveSeam(permute(ReducedInputImage, [2,1,3]), SeamHor);
ReducedGrayImage = RemoveSeam(permute(ReducedGrayImage, [2,1,3]), SeamHor);
ReducedGradMean = RemoveSeamGradient(permute(ReducedGradMean,[2,1,3]), SeamHor);
ReducedInputImage = permute(ReducedInputImage, [2,1,3]);
ReducedGradMean = permute(ReducedGradMean, [2,1,3]);
ReducedGrayImage = permute(ReducedGrayImage, [2,1,3]);
end

%Calculating Forward energy map
function [EnergyMap] = CalcEnergyMapForward(GradMean,ReducedGrayImage)
    [rows, cols] = size(GradMean);
  for i = 1:1:rows
    for j = 1:1:cols
      if(i== 1)
        EnergyMap(1,:) = GradMean(1,:);
      else if(j == 1)
        EnergyMap(i,j) = GradMean(i,j)+ min((EnergyMap(i-1,j) +
          costVertical(i,j,ReducedGrayImage)), (EnergyMap(i-1,j+1)+
          costRight(i,j,ReducedGrayImage)));
      elseif(j == cols)
        EnergyMap(i,j) = GradMean(i,j)+ min((EnergyMap(i-1,j-1)+
          costLeft(i,j,ReducedGrayImage)), (EnergyMap(i-1,j)+
          costVertical(i,j,ReducedGrayImage)));
      else
        EnergyMap(i,j) = GradMean(i,j)+ min(min((EnergyMap(i-1,j-1)+
          costLeft(i,j,ReducedGrayImage)), (EnergyMap(i-1,j)+
          costVertical(i,j,ReducedGrayImage))), (EnergyMap(i-1,j+1)+
          costRight(i,j,ReducedGrayImage)));
      end
    end
  end
end

%Additional cost functions for forward energy map
function [value] = costLeft(i,j,ReducedGrayImage)
  [cols] = size(ReducedGrayImage,2);
  if(j==cols)
    value = abs(ReducedGrayImage(i-1,j)-ReducedGrayImage(i,j-1));
  else
    value = abs(ReducedGrayImage(i,j+1)-ReducedGrayImage(i,j-1))+abs(ReducedGrayImage(i-1,j)-ReducedGrayImage(i,j-1));
  end
end

function [value] = costRight(i,j,ReducedGrayImage)
  if(j==1)
    value = abs(ReducedGrayImage(i-1,j)-ReducedGrayImage(i,j+1));
  else
    value = abs(ReducedGrayImage(i,j+1)-ReducedGrayImage(i,j-1))+abs(ReducedGrayImage(i-1,j)-ReducedGrayImage(i,j+1));
  end
end

function [value] = costVertical(i,j,ReducedGrayImage)
  [cols] = size(ReducedGrayImage,2);
  if(j==1 || j==cols)
    value = 0;
  else
    value = abs(ReducedGrayImage(i,j+1)-ReducedGrayImage(i,j-1));
  end
end

%Finding Seam from Energy Map
function[Seam] = findSeam(EnergyMap)
[rows, cols] = size(EnergyMap);
Seam = zeros(rows,1);
for k = 1:1:rows
    if(k==1)
        [Value, Seam(rows)] = min(EnergyMap(rows,:)); 
    else
        if(Seam(rows-k+2) == 1)
             [Value, temp] = min(EnergyMap(rows-k+1,1:2));
             Seam(rows-k+1)= temp;
        elseif(Seam(rows-k+2) == cols)
             [Value, temp] = min(EnergyMap(rows-k+1,cols-1:cols));
             Seam(rows-k+1)= Seam(rows-k+2)-2 + temp;
        else
        [Value, temp] = min(EnergyMap(rows-k+1,Seam(rows-k+2)-1:1:Seam(rows-k+2)+1));
        Seam(rows-k+1)= Seam(rows-k+2)-2 + temp;
        end
    end
end
end

%Removing seam from image
function [ReducedImage] = RemoveSeam(inputImage, Seam)
[rows, cols, dim] = size(inputImage);
ReducedImage = zeros(rows,cols-1, dim);
%imshow(inputImage);
for k=1:1:dim
    for i=1:1:rows
        for j=1:1:cols-1
            if(j < Seam(i))
                ReducedImage(i,j,k) = inputImage(i,j,k);
            elseif(j >= Seam(i))
                ReducedImage(i,j,k) = inputImage(i,j+1,k);
            end                
        end
    end
        
end
end

%Remove Seam from gradient image. We add importance diffusion for gradient
%image in this funciton so we calculated it seperately.
function [ReducedGradMean] = RemoveSeamGradient(GradMean, Seam)
[rows, cols, dim] = size(GradMean);
ReducedGradMean = zeros(rows,cols-1, dim);
for i=1:1:rows
    for j=1:1:cols-1
        %%% Importance Diffusion Starts
%         if(j == Seam(i)-1)
%             GradMean(i,j) = GradMean(i,j) + GradMean(i,Seam(i))/8;
%         end
%         if(j == Seam(i))
%             GradMean(i,j+1) = GradMean(i,j+1) + GradMean(i,Seam(i))/8;
%         end
        %%Importance Diffusion Ends
        if(j < Seam(i))
            ReducedGradMean(i,j) = GradMean(i,j);
        elseif(j >= Seam(i))
            ReducedGradMean(i,j) = GradMean(i,j+1);
        end                
    end
end
end
