import glob
import numpy as np
import pickle
import fit_image_data
import ehtim as eh

#imgfiles=sorted(glob.glob('/Users/jdexter/analyses/eht/ipole_images/MAD_a0/MAD_a0/image_MAD_192_a*.dat'))
imgfiles=sorted(glob.glob('/Users/jdexter/analyses/eht/ipole_images/MAD_a+0.94/MAD_a+0.94/image_MAD*_1_20.dat'))
#imgfiles=sorted(glob.glob('/Users/jdexter/Desktop/ipole_images/SANE_a0/SANE_a0/image_SANE_192_a*.dat'))
obsfile = '/Users/jdexter/analyses/eht/er5/hops_hi/hops_3601_M87+netcal.uvfits'
name='mada94rhigh20_hops_hi_3601_M87'

first_null = []; bump_amp = []; res = []; obs_list = []; best_img = []
nfiles = len(imgfiles)
step=1
print nfiles
for i in range(nfiles/step):
    imgfile=imgfiles[i*step]
    print 'imgfile: ',imgfile
    first_null_t, bump_amp_t, res_t, obs_t, best_img_t, obs_perfect_t = fit_image_data.run(obsfile,imgfile,'ipole',eh.RADPERUAS,name+'_'+str(i*20),fullfit=True,N=10)
    first_null.append(first_null_t); bump_amp.append(bump_amp_t); res.append(res_t); obs_list.append(obs_t)
    best_img.append(best_img_t.copy())
np.save(name+'_null.npy',np.array(first_null)); np.save(name+'_bump.npy',np.array(bump_amp))
pickle.dump(res,open(name+'_res.p','wb')); pickle.dump(best_img,open(name+'_imgs.p','wb'))
