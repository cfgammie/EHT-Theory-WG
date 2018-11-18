import matplotlib
matplotlib.rcParams['backend'] = "Qt4Agg"
import sys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm, colors, rcParams
from mpl_toolkits.axes_grid1 import make_axes_locatable
from matplotlib.colors import LogNorm
from matplotlib.ticker import MultipleLocator
from astropy.io import fits

## @package my_plot
# my_plot contains the main plotting functions which are used by the rest of the modules.
# Specifically, it contains functions plot VLBI observables fromm GRMHD simulations.

matplotlib.rcParams['mathtext.fontset'] = 'stix'
matplotlib.rcParams['font.family'] = 'STIXGeneral'


def plot_image(array, fig, ax1, pixel, norm=True, scale='lin', font=16, colorbar=True, lim_lin=np.array([0,1]), norm_num=1, lim_log=False, horz=False, x_label=True, y_label=True, colorbar_ticks='set'):
    pixel=pixel/(2.778e-10)
    xx = np.shape(array)[0]
    mid     = (np.float(xx-1))/2.0
    extent = mid*pixel
    
    if norm == True: array = array/(np.max(array))
    
    plt.set_cmap('gnuplot2')
    array = np.fliplr(array)
    if scale == 'lin':
        im1 = ax1.imshow(array, extent=[extent, -extent, -extent, extent],vmin=lim_lin[0], vmax=lim_lin[1], origin='lower', interpolation='bilinear')
        if colorbar==True:
            divider1 = make_axes_locatable(ax1)
            cax1     = divider1.append_axes("right", size="7%", pad=0.05)
            if colorbar_ticks == 'auto': cbar1    = plt.colorbar(im1, cax=cax1)
            else: cbar1    = plt.colorbar(im1, cax=cax1, ticks=[0,0.2,0.4,0.6,0.8,1])
            cbar1.ax.tick_params(labelsize=font)
    if scale == 'log':
        if type(lim_log) ==bool: im1=ax1.imshow(array, extent=[extent, -extent, -extent, extent], norm=LogNorm(), origin='lower', interpolation='bilinear')
        else: im1=ax1.imshow(array, extent=[extent, -extent, -extent, extent], norm=LogNorm(vmin=lim_log[0], vmax=lim_log[1]), origin='lower', interpolation='bilinear')
        
        if colorbar==True:
            divider1 = make_axes_locatable(ax1)
            cax1     = divider1.append_axes("right", size="7%", pad=0.05)
            cbar1    = plt.colorbar(im1, cax=cax1)
            cbar1.ax.tick_params(labelsize=font)
    ax1.tick_params(axis='both', which='major', labelsize=font, color='w',width=1.5)
    ax1.set_xlim([ -extent,extent])
    ax1.set_ylim([-extent, extent])                
                
    if x_label==True: ax1.set_xlabel('RA ($\mu$arcsec)',fontsize=font)
    if y_label==True: ax1.set_ylabel('DEC ($\mu$arcsec)',fontsize=font)
    return(0)
    
    
file_name = sys.argv[1]

hdulist = fits.open(file_name)
hdu     = hdulist[0]
array   = hdu.data
pixel   = np.abs(hdu.header['CDELT1'])
hdulist.close()

fig,(ax1) = plt.subplots(1,1)    
fig.set_size_inches(4,4)

plot_image(array, fig,ax1,pixel)

if len(sys.argv) ==3:
    fig.savefig(sys.argv[2], bbox_inches='tight')
    plt.close(fig)
else:plt.show()


