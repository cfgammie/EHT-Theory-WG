#!/usr/bin/env python
import string,math,sys,fileinput,glob,os,time
import matplotlib.pyplot as plt
import numpy as np
import numpy.ma as ma

#use astropy for fits and source locations
from astropy.time import Time as aTime
from astropy.coordinates import SkyCoord
import astropy.io.fits as fits

def ipole2fits(filename,freq,source,date,history,user,plot=1,show=False):

    dim=160
    ##some definitions
    DEGREE = 3.141592653589/180.0
    HOUR = 15.0*DEGREE
    RADPERAS = DEGREE/3600.0
    RADPERUAS = RADPERAS*1.e-6

    #get file name
    filename_load=filename
    tmp=filename.split('/')
    filename=tmp[1]

    #get source postion
    loc=SkyCoord.from_name(source)

    #get RA and DEC in degree
    ra=loc.ra.deg
    dec=loc.dec.deg

    #convert date to mjd (modified julian date)
    modjuldate=(aTime(date)).mjd

        
    print(filename_load)
    data=((np.loadtxt(filename_load,skiprows = 0, usecols = (2))).reshape(dim,dim)).T

    header = fits.Header()
    header['AUTHOR'] =  user
    header['OBJECT'] =  source
    header['CTYPE1'] = 'RA---SIN'
    header['CTYPE2'] = 'DEC--SIN'
    header['CDELT1'] =  -1*RADPERUAS/DEGREE
    header['CDELT2'] =  1*RADPERUAS/DEGREE
    header['OBSRA'] = ra
    header['OBSDEC'] = dec
    header['FREQ'] = freq
    header['MJD'] = float(modjuldate)
    header['TELESCOP'] = 'VLBI'
    header['BUNIT'] = 'JY/PIXEL'
    header['STOKES'] = 'I'
    header['HISTORY'] = history
    hdu = fits.PrimaryHDU(data, header=header)
    hdulist = [hdu]

    hdulist = fits.HDUList(hdulist)

    # Save fits
    hdulist.writeto('FITS/ipole_%s_%i_%iGHz.fits' %(filename[:-4],dim,(freq/1e9)), overwrite=True)


    print(data.shape)
    


def BHOSS2fits(filename,freq,source,date,history,user,plot=1,show=False):

    ##some definitions
    DEGREE = 3.141592653589/180.0
    HOUR = 15.0*DEGREE
    RADPERAS = DEGREE/3600.0
    RADPERUAS = RADPERAS*1.e-6

    #get file name
    filename_load=filename
    tmp=filename.split('_')

    #get source postion
    loc=SkyCoord.from_name(source)

    #get RA and DEC in degree
    ra=loc.ra.deg
    dec=loc.dec.deg

    #convert date to mjd (modified julian date)
    modjuldate=(aTime(date)).mjd
    
    #get BHOSS GRRT file based on Z. Younsi script
    #--> needs to be updated once the BHOSS header is modified!!!!
    #
    
    # First header line: [image width, offset, resolution, # of observational frequencies]
    header_1 = np.genfromtxt(filename, max_rows = 1)
    # Second header line: [observational time, inclination, BH spin parameter, Luminosity F_nu correction to erg Hz, Jansky correction to Jy Hz ]
    header_2 = np.genfromtxt(filename, skip_header = 1, max_rows = 1)
    # Third header line: observational frequencies of interest
    header_3 = np.genfromtxt(filename, skip_header = 2, max_rows = 1)


    width       = header_1[0]
    offset      = header_1[1]
    M           = int(header_1[2])
    s1          = width + offset
    s2          = 2*width/(M - 1)
    N_obs_freqs = header_1[3]

    time        = header_2[0]
    inclination = header_2[1]
    phi         = header_2[2]
    spin        = header_2[3]
    L_corr      = header_2[4]
    Jansky_corr = header_2[5]

    
    if source=='SgrA*':
        Micro_Arcsecond_Corr = 5.04975 # New value based on Boehle et al. 2016
    if source=='M 87':
        Micro_Arcsecond_Corr = 3.622197344489511 # New value based from Yosuke for M87

    width_scaled         = Micro_Arcsecond_Corr*width   # Scale from r_g to micro-arcseconds


    #find select frequency within computed freqs
    ind=np.where(header_3==freq)

    if len(ind[0])<1:
        sys.exit('EXIT:freq not in GRRT file. Available freqs are %s' %str((header_3)))
    else:
        
        freq_ID=ind[0][0]+3
    print header_3
    print header_3[freq_ID-3]
    # Now read in all image data
    ascii2 = np.loadtxt(filename_load, skiprows = 3, usecols = (0, 1, freq_ID))
    data2=ascii2.reshape([M, M, 3])

    # Convert from indices to (alpha,beta), in units of r_g, on the image plane
    x = -s1 + s2*(data2[:,:,0] - 1)
    y = -s1 + s2*(data2[:,:,1] - 1)


    # Convert (alpha,beta) into micro-arcseconds
    x = Micro_Arcsecond_Corr*x
    y = Micro_Arcsecond_Corr*y

    xmax    = np.amax(x)
    xmin    = np.amin(x)
    ymax    = np.amax(y)
    ymin    = np.amin(y)

    #flux in Jansky
    jansky=(data2[:,:,2]*Jansky_corr)

    #create pixel size
    dxorg=(xmax-xmin)/x.shape[0]
    dyorg=(ymax-ymin)/y.shape[0]
    dim=x.shape[0]
    print('image resolution %f' %(dxorg))
    print('org res. total flux:', ma.sum(jansky), 'max flux:',ma.amax(jansky))


    #create fits file

    # Create header and fill in some values
    header = fits.Header()
    header['AUTHOR'] =  user
    header['OBJECT'] =  source
    header['CTYPE1'] = 'RA---SIN'
    header['CTYPE2'] = 'DEC--SIN'
    header['CDELT1'] = -dxorg*RADPERUAS/DEGREE
    header['CDELT2'] =  dxorg*RADPERUAS/DEGREE
    header['OBSRA'] = ra
    header['OBSDEC'] = dec
    header['FREQ'] = freq
    header['MJD'] = float(modjuldate)
    header['TELESCOP'] = 'VLBI'
    header['BUNIT'] = 'JY/PIXEL'
    header['STOKES'] = 'I'
    header['HISTORY'] = history
    hdu = fits.PrimaryHDU(jansky, header=header)
    hdulist = [hdu]

    hdulist = fits.HDUList(hdulist)

    # Save fits
    tmp=filename.split('/')
    outname=tmp[-1]
    hdulist.writeto('FITS/%s_%i_%iGHz.fits' %(outname[:-4],dim,(freq/1e9)), overwrite=True)

    
    if plot==1:
            
            #create image (normalised)
            fonts=12
            cmap='cubehelix'
            fig=plt.figure(figsize=(7,8))
            plt.subplots_adjust(left=0.15, bottom=0.1, right=0.95, top=0.85, wspace=0.00001, hspace=0.00001)

            ax=fig.add_subplot(111,frame_on='True',aspect='equal',axisbg='k')
            #i1=ax.imshow(jansky/ma.amax(jansky),origin='lower', vmin=0,vmax=1,extent=[xmax, xmin, ymin, ymax], interpolation="bicubic",cmap=cmap )
            i1=ax.imshow(ma.log10(jansky),origin='lower',vmin=-10,vmax=ma.log10(0.05),extent=[xmax, xmin, ymin, ymax], interpolation="bicubic",cmap=cmap )

            
            ax.annotate(r'$\mathrm{%s}$' %(source), xy=(0.1, 0.91),xycoords='axes fraction', fontsize=18,
                            horizontalalignment='left', verticalalignment='bottom', color='w')
            ax.annotate(r'$\mathrm{{\nu=%i\,GHz}}$' %(freq/1e9), xy=(0.1, 0.81),xycoords='axes fraction', fontsize=18,
                            horizontalalignment='left', verticalalignment='bottom', color='w')
    


            #set axis
            plt.xlabel('relative R.A [$\mu$as]', fontsize=fonts)
            plt.ylabel('relative DEC [$\mu$as]', fontsize=fonts)


            #set position of colorbar
            p1=ax.get_position().get_points().flatten()
            ax11 = fig.add_axes([p1[0], p1[3], p1[2]-p1[0], 0.02])
            t1=plt.colorbar(i1,cax=ax11,orientation='horizontal',format='%1.1f')
            #t1.set_label(r'$\rm{S/S}_{\rm{max}}$',fontsize=fonts+4,labelpad=-65)
            t1.set_label(r'$\log_{10}(S)\,\mathrm{[Jy/pixel]}$',fontsize=fonts+4,labelpad=-65)
            t1.ax.xaxis.set_ticks_position('top')
            t1.ax.tick_params(labelsize=fonts) 


            #set xmax left and ticks
            ax.set_xlim(xmax,xmin)
            ax.set_ylim(ymin,ymax)
            ax.yaxis.set_major_locator(plt.MultipleLocator(20))
            ax.yaxis.set_minor_locator(plt.MultipleLocator(5))
            ax.xaxis.set_major_locator(plt.MultipleLocator(20))
            ax.xaxis.set_minor_locator(plt.MultipleLocator(5))

            for spine in ax.spines.values():
                spine.set_edgecolor('w')

            #settings for axes and tickmarks
            for label in ax.xaxis.get_ticklabels():
                label.set_fontsize(fonts)

            for label in ax.yaxis.get_ticklabels():
                label.set_fontsize(fonts)
            for tick in ax.xaxis.get_major_ticks():
                tick.tick1line.set_color('w')
                tick.tick2line.set_color('w')
                tick.tick1line.set_markersize(14)
                tick.tick2line.set_markersize(14)
            for tick in ax.xaxis.get_minor_ticks():
                tick.tick1line.set_color('w')
                tick.tick2line.set_color('w')
                tick.tick1line.set_markersize(10)
                tick.tick2line.set_markersize(10)
            for tick in ax.yaxis.get_major_ticks():
                tick.tick1line.set_color('w')
                tick.tick2line.set_color('w')
                tick.tick1line.set_markersize(14)
                tick.tick2line.set_markersize(14)
            for tick in ax.yaxis.get_minor_ticks():
                tick.tick1line.set_color('w')
                tick.tick2line.set_color('w')
                tick.tick1line.set_markersize(10)
                tick.tick2line.set_markersize(10)

            #save image
            plt.savefig('IMAGE/%s_%i_%iGHz.pdf' %(source,dim,(freq/1e9)),dpi=150, bbox_inches='tight', pad_inches = 0.04)
            if show==True:
                plt.show()

    return {'image':jansky,'dx':dxorg,'dy':dyorg}


#########BHOSS to fits routines#######
##directory structure should be:
##GRRT --> GRRT files are stored here
##FITS --> FITS file will be stored here
##IMAGE --> images will be stored here

###get file
files=sys.argv[1]
code=sys.argv[2]

####include your user name in the fits header
user='C.M. Fromm'

###history include your model settings, all details you want to share 
history='''GRMHD: BHAC, Kerr, a=0.9375, 3D, 192x192x192 GRRT: BHOSS, averaged, t=8900-10000, Rhigh=10, sigma_cut=10, -hu_t=1.02, i=60'''

###provide source name
source='M 87'

###observing frequency in Hz
freq=230e9

###obs date in yyyy-mm-dd
date='2018-04-06'


if code=='BHOSS':
    BHOSS2fits(i,230e9,source,data,history,user,plot=1,show=False)
elif code=='Ipole':
    ipole2fits(i,230e9,source,data,history,user,plot=1,show=False)
else:
    sys.exit('Code not yet included')
