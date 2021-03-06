function [volume,interpCube] = interpolateAvg4D(observations,step,interpCube)
% selectFrames: ECG gating using the recorded ECG signal. Given an initial
% frame and the number of frames per sweep location, this function chooses 
% frames such that they are picked at the same location in the heart cycle. 
% This function is intended to be used with imageStitchingECG_v2.m.
% 'imLoc' variable is created in that script, and it is the directory
% location where the frame grabs are located.
%
% observations : pixel locations and intensities
% step : spacing between interpolated pixels (assume isometric)
% volume : interpolated volume

xyz = observations(:,1:3);
c = observations(:,4);

if(~isempty(interpCube))
    Xd = interpCube.Xd;
    Yd = interpCube.Yd;
    Zd = interpCube.Zd;

    x_disc = interpCube.x_disc;
    y_disc = interpCube.y_disc;
    z_disc = interpCube.z_disc;
else
    % bounds
    xyzmin = floor(min(xyz)); % in mm
    xyzmax = ceil(max(xyz)); % in mm
    xyzdiff = xyzmax - xyzmin;
    xyzdiffpctg = xyzdiff*0.2;
    xyzmin = floor(xyzmin - xyzdiffpctg);
    xyzmax = ceil(xyzmax + xyzdiffpctg);
    
    % bins
    x_disc = (xyzmin(1)):step:(xyzmax(1));
    y_disc = (xyzmin(2)):step:(xyzmax(2));
    z_disc = (xyzmin(3)):step:(xyzmax(3));
    
    [Xd,Yd,Zd] = meshgrid(x_disc,y_disc,z_disc);
    
    interpCube.Xd = Xd;
    interpCube.Yd = Yd;
    interpCube.Zd = Zd;

    interpCube.x_disc = x_disc;
    interpCube.y_disc = y_disc;
    interpCube.z_disc = z_disc;
end

nPoints = size(c,1); % number of observations
interpSize = size(Xd); % interpolated volume dimensions

% create containers
intensitySumCube = zeros(interpSize);
pointCountsCube = zeros(interpSize);

% Go through all observations and assign each to a location in the cube
for i = 1:nPoints
    obsX = xyz(i,1);
    obsY = xyz(i,2);
    obsZ = xyz(i,3);
    obsC = c(i,1);
    
    % figure out membership
    idxX = ((obsX-step/2.0) <= x_disc) & ((obsX+step/2.0) >= x_disc);
    idxY = ((obsY-step/2.0) <= y_disc) & ((obsY+step/2.0) >= y_disc);
    idxZ = ((obsZ-step/2.0) <= z_disc) & ((obsZ+step/2.0) >= z_disc);
    
    % add intensity to sum
    intensitySumCube(idxY,idxX,idxZ) = intensitySumCube(idxY,idxX,idxZ) + obsC;
    % increment corresponding counter
    pointCountsCube(idxY,idxX,idxZ) = pointCountsCube(idxY,idxX,idxZ) + 1;
end

% compute average
avgIntensityCube = intensitySumCube./pointCountsCube;

% take the non-inf non-NaN indices and do convex hull
goodIdx = isfinite(avgIntensityCube);
ptsX = Xd(goodIdx);
ptsY = Yd(goodIdx);
ptsZ = Zd(goodIdx);
ptsxyz = [ptsX(:),ptsY(:),ptsZ(:)];

%% Find convex hull and pad with zeros
XYZd = [Xd(:),Yd(:),Zd(:)]; % test points

%convHullIdx = convhulln(ptsxyz,'simplify', true);

% find blob
%in = inhull(XYZd,ptsxyz,convHullIdx);
in = inhull(XYZd,ptsxyz);
in3 = reshape(in,size(Xd,1),size(Xd,2),size(Xd,3));

% might need to dilate by one
% W = 2;
% dilatedEdges = imdilate(in3,strel('square', W) );

volume = avgIntensityCube;
volume(~in3) = 0;

if(ismac) % not engouh GPU RAM
    fprintf('THIS IS A MAC: NOT ENOUGH GPU RAM, or NO CUDA: USE SLOW INTERP\n');
    CdZeroed = griddata(xyz(:,1),xyz(:,2),xyz(:,3),c,Xd(:),Yd(:),Zd(:));
    CdZeroed = reshape(CdZeroed,size(Xd,1),size(Xd,2),size(Xd,3));
    
    volume = CdZeroed;
end

end