import glob
import numpy as np
import pickle
import fit_image_data
import ehtim as eh

#imgfiles=sorted(glob.glob('/Users/jdexter/Desktop/ipole_images/MAD_a0/MAD_a0/image_MAD_192_a*.dat'))
imgfiles=sorted(glob.glob('/Users/jdexter/Desktop/ipole_images/SANE_a0/SANE_a0/image_SANE_192_a*.dat'))

obsfile = '/Users/jdexter/analyses/eht/er5/hops_hi/hops_3598_M87+netcal.uvfits'

first_null = []; bump_amp = []; res = []; obs_list = []; best_img = []
nfiles = len(imgfiles)
step=50
for i in range(nfiles/step):
    imgfile=imgfiles[i*step]
    first_null_t, bump_amp_t, res_t, obs_t, best_img_t, obs_perfect_t = fit_image_data.run(obsfile,imgfile,'ipole',eh.RADPERUAS,'sanea0_test_'+str(i*20),fullfit=False,N=10)
    first_null.append(first_null_t); bump_amp.append(bump_amp_t); res.append(res_t); obs_list.append(obs_t)
    best_img.append(best_img_t.copy())
np.save('sanea0_m87_test_null.npy',np.array(first_null)); np.save('sanea0_m87_test_bump.npy',np.array(bump_amp))
pickle.dump(res,open('sanea0_m87_test_res.p','wb')); pickle.dump(best_img,open('sanea0_m87_test_imgs.p','wb'))
