import ehtim as eh
import numpy as np
from scipy.ndimage import rotate

def setup_pol_im(obs,ivals,pixel,fluxscl=0.5,pa=-90.):
    sim_img = ivals.copy()
    pol=['I','Q','U','V']
    im = eh.image.Image(sim_img[0], psize=pixel,ra=obs.ra, dec=obs.dec,rf=obs.rf,pol_prim='I')
    for i in range(3):
       im.add_pol_image(sim_img[i+1],pol[i+1])
    fluxfac=fluxscl/im.total_flux()
    im = im.rotate(pa/180.*np.pi)
    im.imvec *= fluxfac
    im.qvec *= fluxfac
    im.uvec *= fluxfac
    im.vvec *= fluxfac
    return im

def setup_obs(obs,sefdfac=1.,fluxscl=0.8,selfcal_lmt=False):
# try to simulate some data
#obs = eh.obsdata.load_uvfits('/Users/jdexter/analyses/eht/er5/hops_lo/3601/hops_3601_M87+netcal.uvfits')
    if selfcal_lmt==False:
        obs = obs.flag_sites(['SR']) #remove SMA reference antenna, if necessary  
        eht = eh.array.load_txt('/Users/jdexter/code/eht-imaging/arrays/EHT2017.txt')

# Populate the array table
        t_obs = list(obs.tarr['site'])
        t_eht = list(eht.tarr['site'])
        t_conv = {'AA':'ALMA','AP':'APEX','SM':'SMA','JC':'JCMT','AZ':'SMT','LM':'LMT','PV':'PV','SP':'SPT'}
        for t in t_conv.keys():
            if t in obs.tarr['site']:
                for key in ['fr_par','fr_elev','fr_off','sefdl','sefdr']:
                    obs.tarr[key][t_obs.index(t)] = eht.tarr[key][t_eht.index(t_conv[t])]*sefdfac

# Make sure that there are no zero SEFDs unless intended
        if sefdfac > 0.:
            for j in range(len(obs.tarr)):
                if obs.tarr[j]['sefdl'] == 0: obs.tarr[j]['sefdl'] = 1e4
                if obs.tarr[j]['sefdr'] == 0: obs.tarr[j]['sefdr'] = 1e4
    else:
#       self-cal LMT to a Gaussian
        LZ_gauss = 50*eh.RADPERUAS   # Gaussian FWHM for initial self-calibration of the LMT-SMT baseline
        npix = 128               # Number of pixels in the reconstruction
        fov = 200*eh.RADPERUAS  # The FOV of the reconstruction
        zbl=fluxscl
        emptyprior = eh.image.make_square(obs, npix, fov)
        gausspriorLMT = emptyprior.add_gauss(zbl, (LZ_gauss, LZ_gauss, -np.pi/4, 0, 0))
        obs_sc = obs.copy()
        for ii in range(3):
            caltab = eh.self_cal.self_cal(obs_sc.flag_uvdist(uv_max=2e9), gausspriorLMT, 
                                          sites=['LM','LM'], method='vis', ttype='nfft', 
                                          processes=4, caltable=True, gain_tol=1.0)
            obs_sc = caltab.applycal(obs_sc, interp='nearest', extrapolate=True)
        obs = obs_sc.copy()
    return obs

# TAKEN FROM eht-imaging IMAGE.PY
# here just corrupt visibility data after it is loaded into an obs object
# could for example replace the data points with those from a model
        # Jones Matrix Corruption & Calibration
TAUDEF = 0.1
GAINPDEF = 0.1
DTERMPDEF = 0.05
GAIN_OFFSETS = {'AA':0.15, 'AP':0.015, 'AZ':0.15, 'LM':0.5, 'PV':0.15, 'SM':0.15, 'JC':0.15, 'SR':0.0}
GAINS = {'AA':0.05, 'AP':0.05, 'AZ':0.05, 'LM':0.25, 'PV':0.05, 'SM':0.05, 'JC':0.05, 'SR':0.0}


def sim_vis(obs_in,add_th_noise=True,
            opacitycal=True, ampcal=True, phasecal=True, dcal=True, frcal=True,
            stabilize_scan_phase=False, stabilize_scan_amp=False, 
            jones=False, inv_jones=False,
            tau=TAUDEF, taup=TAUDEF,
            gain_offset=GAIN_OFFSETS, gainp=GAINS,
            dtermp=DTERMPDEF, dterm_offset=DTERMPDEF, seed=False,selfcal_lmt=False):
    obs = obs_in.copy()
    if jones:
        obsdata = simobs.add_jones_and_noise(obs, add_th_noise=add_th_noise,
                                             opacitycal=opacitycal, ampcal=ampcal,
                                             phasecal=phasecal, dcal=dcal, frcal=frcal,
                                             stabilize_scan_phase=stabilize_scan_phase,
                                             stabilize_scan_amp=stabilize_scan_amp,
                                             gainp=gainp, taup=taup, gain_offset=gain_offset,
                                             dtermp=dtermp,dterm_offset=dterm_offset, seed=seed)

        obs =  ehtim.obsdata.Obsdata(obs.ra, obs.dec, obs.rf, obs.bw, obsdata, obs.tarr,
                                         source=obs.source, mjd=obs.mjd, polrep=obs_in.polrep,
                                         ampcal=ampcal, phasecal=phasecal, opacitycal=opacitycal, dcal=dcal, frcal=frcal,
                                         timetype=obs.timetype, scantable=obs.scans)

        if inv_jones:
            obsdata = simobs.apply_jones_inverse(obs, opacitycal=opacitycal, dcal=dcal, frcal=frcal)

            obs =  ehtim.obsdata.Obsdata(obs.ra, obs.dec, obs.rf, obs.bw, obsdata, obs.tarr,
                                     source=obs.source, mjd=obs.mjd, polrep=obs_in.polrep,
                                     ampcal=ampcal, phasecal=phasecal,
                                     opacitycal=True, dcal=True, frcal=True,
                                     timetype=obs.timetype, scantable=obs.scans)
                                             #these are always set to True after inverse jones call
    return obs

# CHANGED removed ttype='direct' since it was causing very weird problems with aliasing to higher correlated flux on all blines
def run(obs,ivals,pixsize,fluxscl=0.5,ampcal=False,phasecal=False,stabilize_scan_phase=True,opacitycal=True,stabilize_scan_amp=True,jones=True,inv_jones=True,dcal=False,frcal=True,add_th_noise=True,dtermp=DTERMPDEF,dterm_offset=DTERMPDEF,sefdfac=1.,gainp=GAINS,gain_offset=GAIN_OFFSETS,onlyvis=False,nonoise=False,scan_avg=True,N=1,pa=-90.,selfcal_lmt=False):
    obs_new = setup_obs(obs,sefdfac=sefdfac,selfcal_lmt=selfcal_lmt,fluxscl=fluxscl)
    obs_list = []
    if onlyvis==True:
        for i in range(N):
            obs_simulated = sim_vis(obs_new,ampcal=ampcal,phasecal=phasecal,stabilize_scan_phase=stabilize_scan_phase,opacitycal=opacitycal,stabilize_scan_amp=stabilize_scan_amp,jones=jones,inv_jones=inv_jones,dcal=dcal,frcal=frcal,add_th_noise=add_th_noise,dtermp=dtermp,dterm_offset=dterm_offset,gain_offset=gain_offset,gainp=gainp,selfcal_lmt=False)
            obs_simulated.add_cphase()
            if scan_avg==True:
                obs_simulated.add_scans()
                obs_simulated = obs_simulated.avg_coherent(0.,scan_avg=True)
            obs_list.append(obs_simulated)
        im=-1.
    else:
        im = setup_pol_im(obs_new,ivals,pixsize,fluxscl=fluxscl,pa=pa)
        for i in range(N):
            if nonoise==False:
                obs_simulated = im.observe_same(obs_new,ampcal=ampcal,phasecal=phasecal,stabilize_scan_phase=stabilize_scan_phase,opacitycal=opacitycal,stabilize_scan_amp=stabilize_scan_amp,jones=jones,inv_jones=inv_jones,dcal=dcal,frcal=frcal,add_th_noise=add_th_noise,dtermp=dtermp,dterm_offset=dterm_offset,gain_offset=gain_offset,gainp=gainp)
            else:
                obs_simulated = im.observe_same_nonoise(obs_new)
#            obs_simulated.add_cphase()
            if scan_avg==True:
                obs_simulated.add_scans()
                obs_simulated = obs_simulated.avg_coherent(0.,scan_avg=True)
            obs_simulated.add_cphase()
            obs_list.append(obs_simulated)
    return obs_list,im,obs_new
