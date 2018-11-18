# Copyright (C) 2017 Lia Medeiros                                                                             
#                                                                                                             
# This file is part of EHTplot.                                                                               
#                                                                                                             
# EHTplot is free software: you can redistribute it and/or modify                                             
# it under the terms of the GNU General Public License as published by                                        
# the Free Software Foundation, either version 3 of the License, or                                           
# (at your option) any later version.                                                                         
#                                                                                                             
# EHTplot is distributed in the hope that it will be useful,                                                  
# but WITHOUT ANY WARRANTY; without even the implied warranty of                                              
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                                               
# GNU General Public License for more details.                                                                
#                                                                                                             
# You should have received a copy of the GNU General Public License                                           
# along with EHTplot.  If not, see <http://www.gnu.org/licenses/>.  

import matplotlib
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm, colors, rcParams
from mpl_toolkits.axes_grid1 import make_axes_locatable
from matplotlib.colors import LogNorm
from matplotlib.ticker import MultipleLocator
import pkg_resources as pr


class Figure:
    def __init__(self, subplots=(1,1), size=None, font=10):
        '''
        subplots is (row, column) but size is (x,y)
        '''
        
        textwidth=7.1
        columnwidth=3.39375
        if size is None:
            w = columnwidth if subplots[1]==1 else textwidth
            size = (w, w/subplots[1]*subplots[0])
        self.fig, axs = plt.subplots(subplots[0], subplots[1], figsize=size)  
        print(axs)
        if subplots[0]==1 and subplots[1]==1:
            axs = np.array([axs])
        print(axs.shape)
        self.axs   = axs.reshape(subplots)
        print(axs.shape)
        self.font  = font
        self.count = 0
        self.size  = size
        
    def plot_image(self,array, pixsize, axis=None, norm=True, scale='lin', colorbar=True, limits=(0,1), colormap='gnuplot2', zoom=False,labelx='auto',labely='auto',lim_log=False):
        '''labelx,labely can be 'auto' which will let the code choose whether to include the label, 
        'force' which will force it to have a label, None which will force it to not have a label,
        or you can set it equal to another string which will force it to have a label equal to that string.
        '''
        n_row = self.axs.shape[0]
        n_col = self.axs.shape[1]
        if axis is None:
            i   = self.count % n_col  # column
            j   = self.count // n_col # row
            ax1 = self.axs[j % n_row,i]
            self.count = self.count + 1
        else:
            ax1 = axis
        font = self.font
        x = array.shape[0]
        if norm == True: array = array/(np.max(array))
        plt.set_cmap(colormap)
        if scale == 'lin':
            im1 = ax1.imshow(array, extent=[-x/2.*pixsize, x/2.*pixsize, -x/2.*pixsize, x/2.*pixsize],vmin=limits[0], vmax=limits[1], origin='lower', interpolation='bilinear', aspect=1)
            if colorbar==True:
                divider1 = make_axes_locatable(ax1)
                cax1     = divider1.append_axes("right", size="7%", pad=0.05)
                cbar1    = plt.colorbar(im1, cax=cax1, ticks=[0,0.2,0.4,0.6,0.8,1])
                cbar1.ax.tick_params(labelsize=font)
                self.fig.set_size_inches(self.size[0],self.size[1]*.8)

        if scale == 'log': 
            im1 = ax1.imshow(array, extent=[-x/2.*pixsize, x/2.*pixsize, -x/2.*pixsize, x/2.*pixsize], LogNorm=(), origin='lower', interpolation='bilinear')
            if colorbar==True:
                divider1 = make_axes_locatable(ax1)
                cax1     = divider1.append_axes("right", size="7%", pad=0.05)
                cbar1    = plt.colorbar(im1, cax=cax1)
                cbar1.ax.tick_params(labelsize=font)
                self.fig.set_size_inches(self.size[0],self.size[1]*.8)
        if labelx is None: pass
        elif labelx =='force': ax1.set_xlabel('X ($GMc^{-2}$)',fontsize=font, labelpad=0)
        elif labelx =='auto': 
            if j==(n_row-1):
                ax1.set_xlabel('X ($GMc^{-2}$)',fontsize=font, labelpad=0)
        else: ax1.set_xlabel(labelx,fontsize=font, labelpad=0)
        
        if labely is None: pass
        elif labely =='force': ax1.set_ylabel('Y ($GMc^{-2}$)',fontsize=font, labelpad=-6)
        elif labely =='auto': 
            if i==0:ax1.set_ylabel('Y ($GMc^{-2}$)',fontsize=font, labelpad=-6)
        else: ax1.set_ylabel(labely,fontsize=font, labelpad=-6)

    
        ax1.tick_params(axis='both', which='major', labelsize=font, color='w',width=1.5)
        if zoom == True:
            r0 = np.sqrt(27)#this gives the BH shadow in units of GM/c**2
            ax1.set_xlim([-r0*2, r0*2])
            ax1.set_ylim([-r0*2, r0*2])
            ax1.set_xticks([-10,-5,0,5,10])
            ax1.set_yticks([-10,-5,0,5,10])

        return(0)
        
    def plot_VA(self,array, pixsize, axis=None, norm=True, scale='lin', colorbar=True, limits=(0,1), colormap='gnuplot2', zoom=False, labelx='auto', labely='auto', btracks=False, lim_log=False):  
        n_row = self.axs.shape[0]
        n_col = self.axs.shape[1]
        if axis is None:
            i   = self.count % n_col  # column
            j   = self.count // n_col # row
            ax1 = self.axs[j % n_row,i]
            self.count = self.count + 1
        else:
            ax1 = axis
        font = self.font
        x = array.shape[0]
        if norm == True: array = array/(np.max(array))
        
        plt.set_cmap(colormap)
        
        if scale == 'lin':
            im1=ax1.imshow(array,extent=[-x/2.*pixsize, x/2.*pixsize, -x/2.*pixsize, x/2.*pixsize],vmin=limits[0], vmax=limits[1], origin='lower', interpolation='bilinear')
            if colorbar==True:
                divider1 = make_axes_locatable(ax1)
                cax1     = divider1.append_axes("right", size="7%", pad=0.05)
                cbar1    = plt.colorbar(im1, cax=cax1, ticks=[0,0.2,0.4,0.6,0.8,1])
                cbar1.ax.tick_params(labelsize=font)
        if scale == 'log': 
            if lim_log ==False: im1=ax1.imshow(array,extent=[-x/2.*pixsize, x/2.*pixsize, -x/2.*pixsize, x/2.*pixsize],norm=LogNorm(),origin='lower', interpolation='bilinear')
            else:               im1=ax1.imshow(array,extent=[-x/2.*pixsize, x/2.*pixsize, -x/2.*pixsize, x/2.*pixsize],norm=LogNorm(vmin=limits[0], vmax=limits[1]),origin='lower', interpolation='bilinear')
            if colorbar==True:
                divider1 = make_axes_locatable(ax1)
                cax1     = divider1.append_axes("right", size="7%", pad=0.05)
                cbar1    = plt.colorbar(im1, cax=cax1)
                cbar1.ax.tick_params(labelsize=font)
                
        if labelx is None: pass
        elif labelx =='force': ax1.set_xlabel('$u$ (G $\lambda$)',fontsize=font, labelpad=0)
        elif labelx =='auto': 
            if j==(n_row-1):
                ax1.set_xlabel('$u$ (G $\lambda$)',fontsize=font, labelpad=0)
        else: ax1.set_xlabel(labelx,fontsize=font, labelpad=0)
        
        if labely is None: pass
        elif labely =='force': ax1.set_ylabel('$v$ (G $\lambda$)',fontsize=font, labelpad=-6)
        elif labely =='auto': 
            if i==0:ax1.set_ylabel('$v$ (G $\lambda$)',fontsize=font, labelpad=-6)
        else: ax1.set_ylabel(labely,fontsize=font, labelpad=-6)
                      
        if btracks == True:
            module   = __name__.split(".")[0]
            data_dir = pr.resource_filename(module, "data/")
            U, V     = np.load(data_dir+'U.npy'), np.load(data_dir+'V.npy')
            U,V      = U*10**(-6),V*10**(-6)
            ax1.scatter( U, V, c='w', s=2, marker='o',edgecolors='none')
            ax1.scatter(-U,-V, c='w', s=2, marker='o',edgecolors='none')
        ax1.tick_params(axis='both', which='major', labelsize=font,color='w',width=1.5)
        
        if zoom == True:
            ext =9.86099511299 # this is equal to 4 *8.3 kpc*c^2/(pi*sqrt(27)*4.31*10^6*solar mass*G)
            ax1.set_xlim([ext,-ext])
            ax1.set_ylim([-ext,ext])
            ax1.set_xticks([-5,0,5])
            ax1.set_yticks([-5,0,5])

        return(0)
        
        
    def plot_VP(self,array, pixsize, axis=None, colorbar=True, limits=(0,1), colormap='hsv', zoom=False, labelx='auto', labely='auto', btracks=False, lim_log=False, units='rad'):  
        if units == 'deg': array = array/360.*2*np.pi
        
        array[np.where(array >  np.pi)] = array[np.where(array >  np.pi)]-2.0*np.pi
        array[np.where(array < -np.pi)] = array[np.where(array < -np.pi)]+2.0*np.pi
        
        n_row = self.axs.shape[0]
        n_col = self.axs.shape[1]
        if axis is None:
            i   = self.count % n_col  # column
            j   = self.count // n_col # row
            ax1 = self.axs[j % n_row,i]
            self.count = self.count + 1
        else:
            ax1 = axis
        font = self.font
        x = array.shape[0]
        
        plt.set_cmap(colormap)
        
        im1=ax1.imshow(array,extent=[-x/2.*pixsize, x/2.*pixsize, -x/2.*pixsize, x/2.*pixsize],vmin=-np.pi, vmax=np.pi, origin='lower', interpolation='bilinear')
        if colorbar==True:
            divider1 = make_axes_locatable(ax1)
            cax1     = divider1.append_axes("right", size="7%", pad=0.05)
            cbar1    = plt.colorbar(im1, cax=cax1)
            cbar1.ax.tick_params(labelsize=font)
                
        if labelx is None: pass
        elif labelx =='force': ax1.set_xlabel('$u$ (G $\lambda$)',fontsize=font, labelpad=0)
        elif labelx =='auto': 
            if j==(n_row-1):
                ax1.set_xlabel('$u$ (G $\lambda$)',fontsize=font, labelpad=0)
        else: ax1.set_xlabel(labelx,fontsize=font, labelpad=0)
        
        if labely is None: pass
        elif labely =='force': ax1.set_ylabel('$v$ (G $\lambda$)',fontsize=font, labelpad=-6)
        elif labely =='auto': 
            if i==0:ax1.set_ylabel('$v$ (G $\lambda$)',fontsize=font, labelpad=-6)
        else: ax1.set_ylabel(labely,fontsize=font, labelpad=-6)
                      
        if btracks == True:
            module   = __name__.split(".")[0]
            data_dir = pr.resource_filename(module, "data/")
            U, V     = np.load(data_dir+'U.npy'), np.load(data_dir+'V.npy')
            U,V      = U*10**(-6),V*10**(-6)
            ax1.scatter( U, V, c='w', s=2, marker='o',edgecolors='none')
            ax1.scatter(-U,-V, c='w', s=2, marker='o',edgecolors='none')
        ax1.tick_params(axis='both', which='major', labelsize=font,color='w',width=1.5)
        
        if zoom == True:
            ext =9.86099511299 # this is equal to 4 *8.3 kpc*c^2/(pi*sqrt(27)*4.31*10^6*solar mass*G)
            ax1.set_xlim([ext,-ext])
            ax1.set_ylim([-ext,ext])
            ax1.set_xticks([-5,0,5])
            ax1.set_yticks([-5,0,5])
        return(0)

    