raytrace: bmp.o vector.o miscobj.o lights.o textures.o planar.o quadric.o scene.o octree.o raytrace.o xplot/xplot.o
	CC -g -sb -o raytrace bmp.o vector.o miscobj.o lights.o textures.o planar.o quadric.o scene.o octree.o raytrace.o xplot/xplot.o -L/usr/openwin/lib -lX11

bmp.o:		raytrace.h bmp.h bmp.cc
	CC -c -g -sb -o bmp.o bmp.cc

vector.o:	platform.h raytrace.h vector.h vector.cc
	CC -c -g -sb -o vector.o vector.cc

miscobj.o:	platform.h raytrace.h vector.h miscobj.h miscobj.cc
	CC -c -g -sb -o miscobj.o miscobj.cc

lights.o:	platform.h raytrace.h vector.h miscobj.h lights.h lights.cc
	CC -c -g -sb -o lights.o lights.cc

textures.o:	platform.h raytrace.h vector.h miscobj.h textures.h textures.cc
	CC -c -g -sb -o textures.o textures.cc

planar.o:	platform.h raytrace.h vector.h miscobj.h textures.h object.h planar.h planar.cc
	CC -c -g -sb -o planar.o planar.cc

quadric.o:	platform.h raytrace.h vector.h miscobj.h textures.h object.h quadric.h quadric.cc
	CC -c -g -sb -o quadric.o quadric.cc

scene.o:        platform.h raytrace.h vector.h miscobj.h textures.h planar.h scene.h scene.cc
	CC -c -g -sb -o scene.o scene.cc

octree.o:	platform.h raytrace.h vector.h miscobj.h textures.h planar.h octree.h octree.cc
	CC -c -g -sb -o octree.o octree.cc

raytrace.o:	platform.h raytrace.h vector.h miscobj.h lights.h textures.h planar.h scene.h raytrace.cc
	CC -c -g -sb -o raytrace.o raytrace.cc

xplot/xplot.o:	xplot/xplot.c xplot/driver.h xplot/color.h xplot/standard.h xplot/vfork.h xplot/fvect.h xplot/mat4.h xplot/x11twind.h xplot/x11icon.h
	cc -Xs -c -g -I/usr/openwin/include -o xplot/xplot.o xplot/xplot.c

################### Optimized version  #################

fast:	bmpf.o vectorf.o miscobjf.o lightsf.o texturesf.o planarf.o quadricf.o scene.o octreef.o raytracef.o xplot/xplot.o
	CC -fast -o raytracef bmpf.o vectorf.o miscobjf.o lightsf.o texturesf.o planarf.o quadricf.o scene.o octreef.o raytracef.o xplot/xplot.o -L/usr/openwin/lib -lX11

bmpf.o:		raytrace.h bmp.h bmp.cc
	CC -c -fast -o bmpf.o bmp.cc

vectorf.o:	raytrace.h vector.h vector.cc
	CC -c -fast -o vectorf.o vector.cc

miscobjf.o:	raytrace.h vector.h miscobj.h miscobj.cc
	CC -c -fast -o miscobjf.o miscobj.cc

lightsf.o:	raytrace.h vector.h miscobj.h lights.h lights.cc
	CC -c -fast -o lightsf.o lights.cc

texturesf.o:	raytrace.h vector.h miscobj.h textures.h textures.cc
	CC -c -fast -o texturesf.o textures.cc

planarf.o:	raytrace.h vector.h miscobj.h textures.h object.h planar.h planar.cc
	CC -c -fast -o planarf.o planar.cc

quadricf.o:	raytrace.h vector.h miscobj.h textures.h object.h quadric.h quadric.cc
	CC -c -fast -o quadricf.o quadric.cc

octreef.o:	platform.h raytrace.h vector.h miscobj.h textures.h planar.h octree.h octree.cc
	CC -c -fast -o octreef.o octree.cc

raytracef.o:	raytrace.h vector.h miscobj.h lights.h textures.h planar.h raytrace.cc
	CC -c -fast -o raytracef.o raytrace.cc

xplot/xplotf.o:	xplot/xplot.c xplot/driver.h xplot/color.h xplot/standard.h xplot/vfork.h xplot/fvect.h xplot/mat4.h xplot/x11twind.h xplot/x11icon.h
	cc -Xs -c -fast -o xplot/xplotf.o xplot/xplot.c

################### Optimized debugging version  #################

debug:	bmpdf.o vectordf.o miscobjdf.o lightsdf.o texturesdf.o planardf.o quadricdf.o scene.o octreedf.o raytracedf.o
	CC -fast -g -sb -o raytracedf bmpdf.o vectordf.o miscobjdf.o lightsdf.o texturesdf.o planardf.o quadricdf.o scene.o octreedf.o raytracedf.o xplot/xplots.o -L/usr/openwin/lib -lX11

bmpdf.o:		raytrace.h bmp.h bmp.cc
	CC -c -fast -g -sb -o bmpdf.o bmp.cc

vectordf.o:	raytrace.h vector.h vector.cc
	CC -c -fast -g -sb -o vectordf.o vector.cc

miscobjdf.o:	raytrace.h vector.h miscobj.h miscobj.cc
	CC -c -fast -g -sb -o miscobjdf.o miscobj.cc

lightsdf.o:	raytrace.h vector.h miscobj.h lights.h lights.cc
	CC -c -fast -g -sb -o lightsdf.o lights.cc

texturesdf.o:	raytrace.h vector.h miscobj.h textures.h textures.cc
	CC -c -fast -g -sb -o texturesdf.o textures.cc

planardf.o:	raytrace.h vector.h miscobj.h textures.h planar.h planar.cc
	CC -c -fast -g -sb -o planardf.o planar.cc

quadricdf.o:	platform.h raytrace.h vector.h miscobj.h textures.h object.h quadric.h quadric.cc
	CC -c -g -sb -o quadricdf.o quadric.cc

octreedf.o:	platform.h raytrace.h vector.h miscobj.h textures.h planar.h octree.h octree.cc
	CC -c -fast -g -sb -o octreedf.o octree.cc

raytracedf.o:	raytrace.h vector.h miscobj.h lights.h textures.h planar.h raytrace.cc
	CC -c -fast -g -sb -o raytracedf.o raytrace.cc


#####################  Solaris profiling version  ##############################

prof: vectorp.o miscobjp.o lightsp.o texturesp.o planarp.o quadricp.o scenep.o octreep.o raytracep.o xplot/xplot.o
	CC -p -o raytracep vectorp.o miscobjp.o lightsp.o texturesp.o planarp.o quadricp.o scenep.o octreep.o raytracep.o xplot/xplots.o -L/usr/openwin/lib -lX11

vectorp.o:	platform.h raytrace.h vector.h vector.cc
	CC -c -p -o vectorp.o vector.cc

miscobjp.o:	platform.h raytrace.h vector.h miscobj.h miscobj.cc
	CC -c -p -o miscobjp.o miscobj.cc

lightsp.o:	platform.h raytrace.h vector.h miscobj.h lights.h lights.cc
	CC -c -p -o lightsp.o lights.cc

texturesp.o:	platform.h raytrace.h vector.h miscobj.h textures.h textures.cc
	CC -c -p -o texturesp.o textures.cc

planarp.o:	platform.h raytrace.h vector.h miscobj.h textures.h planar.h planar.cc
	CC -c -p -o planarp.o planar.cc

quadricp.o:	platform.h raytrace.h vector.h miscobj.h textures.h object.h quadric.h quadric.cc
	CC -c -g -sb -o quadricp.o quadric.cc

scenep.o:        platform.h raytrace.h vector.h miscobj.h textures.h planar.h scene.h scene.cc
	CC -c -p -o scenep.o scene.cc

octreep.o:	platform.h raytrace.h vector.h miscobj.h textures.h planar.h octree.h octree.cc
	CC -c -p -o octreep.o octree.cc

raytracep.o:	platform.h raytrace.h vector.h miscobj.h lights.h textures.h planar.h scene.h raytrace.cc
	CC -c -p -o raytracep.o raytrace.cc

#####################  Solaris gprofiling version  #############################

gprof: vectorg.o miscobjg.o lightsg.o texturesg.o planarg.o quadricg.o sceneg.o octreeg.o raytraceg.o xplot/xplot.o
	CC -pg -o raytraceg vectorg.o miscobjg.o lightsg.o texturesg.o planarg.o quadricg.o sceneg.o octreeg.o raytraceg.o xplot/xplots.o -L/usr/openwin/lib -lX11

vectorg.o:	platform.h raytrace.h vector.h vector.cc
	CC -c -pg -o vectorg.o vector.cc

miscobjg.o:	platform.h raytrace.h vector.h miscobj.h miscobj.cc
	CC -c -pg -o miscobjg.o miscobj.cc

lightsg.o:	platform.h raytrace.h vector.h miscobj.h lights.h lights.cc
	CC -c -pg -o lightsg.o lights.cc

texturesg.o:	platform.h raytrace.h vector.h miscobj.h textures.h textures.cc
	CC -c -pg -o texturesg.o textures.cc

planarg.o:	platform.h raytrace.h vector.h miscobj.h textures.h planar.h planar.cc
	CC -c -pg -o planarg.o planar.cc

quadricg.o:	platform.h raytrace.h vector.h miscobj.h textures.h object.h quadric.h quadric.cc
	CC -c -g -sb -o quadricg.o quadric.cc

sceneg.o:        platform.h raytrace.h vector.h miscobj.h textures.h planar.h scene.h scene.cc
	CC -c -pg -o sceneg.o scene.cc

octreeg.o:	platform.h raytrace.h vector.h miscobj.h textures.h planar.h octree.h octree.cc
	CC -c -pg -o octreeg.o octree.cc

raytraceg.o:	platform.h raytrace.h vector.h miscobj.h lights.h textures.h planar.h scene.h raytrace.cc
	CC -c -pg -o raytraceg.o raytrace.cc


#####################  Solaris tcov version ##########################

tcov: vectort.o miscobjt.o lightst.o texturest.o planart.o quadrict.o scenet.o octreet.o raytracet.o xplot/xplot.o
	CC -a -o raytracet vectort.o miscobjt.o lightst.o texturest.o planart.o quadrict.o scenet.o octreet.o raytracet.o xplot/xplots.o -L/usr/openwin/lib -lX11

vectort.o:	platform.h raytrace.h vector.h vector.cc
	CC -c -a -o vectort.o vector.cc

miscobjt.o:	platform.h raytrace.h vector.h miscobj.h miscobj.cc
	CC -c -a -o miscobjt.o miscobj.cc

lightst.o:	platform.h raytrace.h vector.h miscobj.h lights.h lights.cc
	CC -c -a -o lightst.o lights.cc

texturest.o:	platform.h raytrace.h vector.h miscobj.h textures.h textures.cc
	CC -c -a -o texturest.o textures.cc

planart.o:	platform.h raytrace.h vector.h miscobj.h textures.h planar.h planar.cc
	CC -c -a -o planart.o planar.cc

quadrict.o:	platform.h raytrace.h vector.h miscobj.h textures.h object.h quadric.h quadric.cc
	CC -c -g -sb -o quadrict.o quadric.cc

scenet.o:        platform.h raytrace.h vector.h miscobj.h textures.h planar.h scene.h scene.cc
	CC -c -a -o scenet.o scene.cc

octreet.o:	platform.h raytrace.h vector.h miscobj.h textures.h planar.h octree.h octree.cc
	CC -c -a -o octreet.o octree.cc

raytracet.o:	platform.h raytrace.h vector.h miscobj.h lights.h textures.h planar.h scene.h raytrace.cc
	CC -c -a -o raytracet.o raytrace.cc

#########################  Send Source  ###################################
# This section copies all source (*.cc, *.h, Makefile) that has changed
# since the last sendsource operation to a directory, & tars & compresses it.
# Run from the rt directory.

/bump/u2/jim/rtsrc.tar.Z:	rtsrc.tar.Z
	cp rtsrc.tar.Z /bump/u2/jim

rtsrc.tar.Z:	rtsrc.tar
	compress rtsrc.tar

rtsrc.tar:		sendsrc/newflag
	tar cvf rtsrc.tar sendsrc/new/*
	
newflag:
