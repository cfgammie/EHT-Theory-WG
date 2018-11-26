#from __future__ import builtins
from __future__ import print_function
import glob
import numpy as np
import pickle
import fit_image_data
import ehtim as eh
import sys

imgbase = sys.argv[1]; ending = sys.argv[2]; imgformat = sys.argv[3]; simname = sys.argv[4]; band = sys.argv[5]; expnum = sys.argv[6]

#imgfiles=sorted(glob.glob('/bd3/eht/M87SimulationLibrary/GRMHD/MAD/a+0.5/192x96x96_IHARM/images/m_1_1_80/image_MAD_a+0.5_1*80.dat'))
imgfiles=sorted(glob.glob(str(imgbase)+'*'+ending))
obsfile = '/bd3/eht/M87SimulationLibrary/EHTdata/er5/hops_'+str(band)+'/hops_'+str(expnum)+'_M87+netcal.uvfits'
#name='mada5_rhigh20_hops_hi_3601_M87_'
name=str(simname)+'_'+'hops_'+str(band)+'_'+str(expnum)+'_'+'M87_'
#imgformat='ipole'
#pixscale=1.
# this pixscale is for M87 with M_BH = 6.2x10^9, D = 16.7 Mpc, fov in M from -21 to 21
#imgformat='grtrans'
#pixscale=0.8
if imgformat=='ipole':
    pixscale=1.
elif imgformat=='grtrans':
    pixscale=0.8

first_null = []; bump_amp = []; gauss_params =[]; res = []; obs_list = []; best_img = []
nfiles = len(imgfiles)
step=1
print(nfiles)
for i in range(int(nfiles/step)):
    imgfile=imgfiles[i*step]
    print('imgfile: ',imgfile)
    first_null_t, bump_amp_t, gauss_params_t, obs_t, res_t, best_img_t, obs_perfect_t = fit_image_data.run(obsfile,imgfile,imgformat,pixscale*eh.RADPERUAS,name+'_'+str(i*step),fullfit=True,N=10,ttype='fast')
    first_null.append(first_null_t); bump_amp.append(bump_amp_t); res.append(res_t); obs_list.append(obs_t); gauss_params.append(gauss_params_t)
    best_img.append(best_img_t.copy())
np.save(name+'_null.npy',np.array(first_null)); np.save(name+'_bump.npy',np.array(bump_amp))
pickle.dump(res,open(name+'_res.p','wb')); pickle.dump(best_img,open(name+'_imgs.p','wb'))
pickle.dump(obs_list,open(name+'_obslist.p','wb'))
np.save(name+'_gaussparams.npy',np.array(gauss_params))
