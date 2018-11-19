# first attempt at "scoring" theory models by simulating data to estimate errors
# record chi^2 values, visamp / cphase plots, model image

import numpy as np
import simulate_obs
import matplotlib.pyplot as plt
import sys
import ehtim as eh
import matplotlib
matplotlib.use('Agg')
matplotlib.rcParams['image.cmap']='inferno'

# sefdfac = 0 is recommended to use actual noise in data
sefdfac=0.
#gain=0.2
N=10

def read_img(imgfile,imgformat):
    if imgformat=='ipole':
        data = np.genfromtxt(imgfile)
        img = data[:,3:].transpose().reshape((4,160,160))
    if imgformat=='fits':
        
    return img

# calculate chi^2 for a polarized model image and data file using eht-imaging to simulate data from the model with gain errors and leakage
# also do rough cut on the idealized data
def run(file,imgfile,imgformat,pixsize,imgname,nflux=4,fluxmin=0.2,fluxmax=0.8,npa=2,pamin=90.,pamax=75.,fullfit=False,N=10):
    img = read_img(imgfile,imgformat)
    obsall = eh.obsdata.load_uvfits(file,remove_nan=True)
    obsall.add_scans()
    fluxscl=fluxmin+(fluxmax-fluxmin)*np.arange(nflux)/(nflux-1.)
    pa=pamin+(pamax-pamin)*np.arange(npa)/(npa-1.)
    first_null = []; bump_amp = []
    visamp_list = []; visamperr_list = []; cphase_list = []; sigmacp_list = []
    chi2 = []; res = []; imglist = []
    for p in pa:
        print p
        obs_sim_perfect,im_p,obs_new_p = simulate_obs.run(obsall,img,pixsize,nonoise=True,pa=p)
        obs_sim_perfect = obs_sim_perfect[0]
        obs_sim_perfect.add_cphase()
# rough cut
        uvd=np.sqrt(obs_sim_perfect.data['u']**2.+obs_sim_perfect.data['v']**2.)
        ruv=np.where(uvd < 8e9)
        current_null = uvd[ruv[0][np.argmin(np.abs(obs_sim_perfect.data['vis'][ruv]))]]
        first_null.append(current_null)
        bump_amp.append(np.max(np.abs(obs_sim_perfect.data['vis'])[uvd > current_null]))
# full fit
        if fullfit==True:
#            res = []; imglist = []
#            obs.add_cphase()
            obs = obsall.avg_coherent(0.,scan_avg=True)
            obs.add_cphase()
#            obs.plotall('uvdist','amp')
#            print np.shape(obs.cphase['cphase']), len(obs.scans)
#            print np.shape(obs_sim_perfect.cphase['cphase']), len(obs_sim_perfect.scans)
            for f in fluxscl:
                obs_sim_perfect,im_p,obs_new_p = simulate_obs.run(obsall,img,pixsize,nonoise=True,pa=p,fluxscl=f,scan_avg=True)
                obs_sim_perfect = obs_sim_perfect[0]
                obs_sim_perfect.add_cphase()
                obs_list,im,obs_new = simulate_obs.run(obs,img,pixsize,sefdfac=sefdfac,N=N,scan_avg=True,pa=p,fluxscl=f)
                visamp,cphase,visamperr,sigmacp,ruv,u,v,ruvmax = get_values(obs_list)
                res.append(calc_likelihood(ruv,visamp,visamperr,cphase,sigmacp,obs,obs_sim_perfect))
                imglist.append(img)
                visamp_list.append(visamp); cphase_list.append(cphase)
                visamperr_list.append(visamperr); sigmacp_list.append(sigmacp)
                chi2.append(res[-1]['chi2'])
    
    if fullfit==True:
        indx=np.argmin(chi2)
#        print fluxscl[indx], chi2, chi2[indx]
        print indx, len(chi2), len(imglist), len(visamp_list), len(visamperr_list), len(cphase_list), len(sigmacp_list)
#            plt.plot(np.sqrt(obs.data['u']**2.+obs.data['v']**2.),np.abs(obs.data['vis']))
        plot_model_data(obs,ruv,visamp_list[indx],visamperr_list[indx],ruvmax,cphase_list[indx],sigmacp_list[indx],imgname+'_'+str(int(p)))
        best_image = imglist[indx]
        print np.shape(best_image)
        image_plot(best_image[0],imgname)
    else:
        best_image = -1.; obs_list = []; res = []
    return first_null,bump_amp,res,obs_list,best_image,obs_sim_perfect

if __name__ == '__main__':
    res,obs_list,best_img = run(sys.argv[1],sys.argv[2],sys.argv[3])

def image_plot(img,imgname,lim=40.,n=80,sat=0.7):
    x = np.linspace(-4.85e-12*lim,4.85e-12*lim,n)
    y = np.linspace(-4.85e-12*lim,4.85e-12*lim,n)
    x,y = np.meshgrid(x*1e9,y*1e9)
    plt.figure(figsize=(5,5))
    print np.shape(img)
    plt.imshow(img,vmax=sat*np.max(img),extent=[lim,-lim,-lim,lim])
    plt.xlabel(r'x ($\mu$as)'); plt.ylabel(r'y ($\mu$as)')
    plt.savefig(imgname+'_model_image.pdf')

# return set of scores
def calc_likelihood(ruv,visamp,visamperr,cphase,sigmacp,lpamp,lpamperr,obs,obs_sim_perfect,cut_ruv=0.5e9,N=10):
    good=np.where(ruv > cut_ruv)[0]
    print cut_ruv, np.min(ruv), len(good), len(ruv)
    chi2_self_amp=np.sum((visamp-obs_sim_perfect.unpack('amp')['amp'])**2./visamperr**2.)/len(visamp)
    diff=cphase-obs_sim_perfect.cphase['cphase']
    diff[diff > 180.]=360.-diff[diff > 180.]
    chi2_self_cphase=np.sum(diff**2./sigmacp**2.)/len(cphase)
    chi2_amp=np.sum((visamp[good]-obs.unpack('amp')['amp'][good])**2./visamperr[good]**2.)/len(good)
    diff=cphase-obs.cphase['cphase']
    diff[diff > 180.]=360.-diff[diff > 180.]
    chi2_cphase=np.sum(diff**2./sigmacp**2.)/len(cphase)
    chi2=(chi2_cphase*len(cphase)+chi2_amp*len(good))/(len(good)+len(cphase))
    chi2_self=(chi2_self_cphase*len(cphase)+chi2_self_amp*len(good))/(len(good)+len(cphase))
# polarized part
    qamp = obs.unpack('qamp')['qamp']; uamp = obs.unpack('uamp')['uamp']
    lpgood=np.where(~np.isnan(lpamp))[0]
    obs_lpamp = np.sqrt(qamp**2.+uamp**2.)[lpgood]
    chi2_lp_amp=np.sum((lpamp[lpgood]-obs_lpamp)**2./lpamperr[lpgood]**2.)/len(lpgood)
    return dict({'chi2_self':chi2_self,'chi2':chi2,'chi2_amp':chi2_amp,'chi2_cphase':chi2_cphase,'chi2_self_cphase':chi2_self_cphase,'chi2_self_amp':chi2_self_amp,'chi2_lp_amp':chi2_lp_amp})

def get_values(obs_list,visamperr_min=0.):
    ruv=np.sqrt(obs_list[0].data['u']**2.+obs_list[0].data['v']**2.)
    visamp_list = []; cphase_list = []
    visamp = []; cphase = []
#visamp=np.array(visamp); cphase=np.array(cphase)
    obs_list[0].add_cphase()
    uu=obs_list[0].data['u']; vv=obs_list[0].data['v']
    uu1=obs_list[0].cphase['u1']; uu2=obs_list[0].cphase['u2']; uu3=obs_list[0].cphase['u3']
    vv1=obs_list[0].cphase['v1']; vv2=obs_list[0].cphase['v2']; vv3=obs_list[0].cphase['v3']
#cphase=obs.cphase['cphase']
    sigmacp=obs_list[0].cphase['sigmacp']
    uvdist1=np.sqrt(uu1**2.+vv1**2.)
    uvdist2=np.sqrt(uu2**2.+vv2**2.)
    uvdist3=np.sqrt(uu3**2.+vv3**2.)
    uvdistmax=np.max(np.array([[uvdist1],[uvdist2],[uvdist3]]),axis=0)[0]
    uvdist=np.sqrt(uu**2.+vv**2.)
    for o in obs_list:
#        o.add_cphase()
#        o.avg_coherent(0.,scan_avg=True)
        visamp_list.append(np.abs(o.data['vis']))
        cphase_list.append(o.cphase['cphase'])
    print np.shape(visamp_list), np.shape(np.std(visamp_list,axis=0)), np.shape(np.median(visamp_list,axis=0))
    visamp=np.median(visamp_list,axis=0); cphase=np.median(cphase_list,axis=0)
    visamperr=np.sqrt((np.std(visamp_list,axis=0)/np.sqrt(N-1.))**2.+visamperr_min**2.)
    sigmacp=np.std(cphase_list,axis=0)/np.sqrt(N-1.)
    return visamp,cphase,visamperr,sigmacp,uvdist,uu,vv,uvdistmax

#def plot_model_data(ruv,model,vis,viserr,plot_name,names=['visamp','cphase','logcamp'],ylabels=['visibility amplitude','closure phase (deg)','log closure amplitude'],xlabels=[r'uv distance (G$\lambda$)',r'maximum uv distance (G$\lambda$)']):
#    for i in range(len(ruv)):
#        plt.figure(figsize=(6,5))
#        plt.errorbar(ruv[i]/1e9,vis[i],viserr[i],marker='o',linestyle='')
#        plt.xlabel(xlabels[i]); plt.ylabel(ylabels[i])
#        plt.plot(ruv[i]/1e9,model[i],marker='o',linestyle='')
#        plt.savefig(plot_name+'_'+names[i]+'_model_data.pdf')

def plot_model_data(o,ruv,visamp,visamperr,uvdistmax,cphase,sigmacp,plotname):
#    o.plotall('uvdist','amp')
    plt.figure(figsize=(5,4))
    plt.errorbar(ruv/1e9,visamp,visamperr,marker='o',linestyle='',zorder=1,label='simulated model')
    print np.shape(ruv), np.shape(o.data['vis'])
    plt.errorbar(o.unpack('uvdist')['uvdist']/1e9,o.unpack('amp')['amp'],o.unpack('sigma')['sigma'],marker='o',linestyle='',label='data',zorder=3)
    plt.axis([-0.5,9,-0.05,0.8])
    plt.legend(loc='upper right')
    plt.xlabel(r'uv distance (G$\lambda$)'); plt.ylabel('visibility amplitude (Jy)')
    plt.savefig(plotname+'_comparison_visamp.pdf',bbox_inches='tight',pad_inches=0.)
    plt.figure(figsize=(5,4))
    plt.errorbar(uvdistmax/1e9,cphase,sigmacp,marker='o',linestyle='',zorder=1,label='simulated model')
#    plt.plot(np.sqrt(o.data['u']**2.+o.data['v']**2.)/1e9,np.abs(obs_sim_scan_perfect.data['vis']),marker='o',linestyle='',zorder=2,label='truth model')
    plt.plot(uvdistmax/1e9,o.cphase['cphase'],marker='o',linestyle='',label='data',zorder=3)
    plt.axis([-0.5,9,-190.,190.])
#legends, handles = plt.
    plt.legend(loc='upper right')
    plt.xlabel(r'max uv distance (G$\lambda$)'); plt.ylabel('closure phase (deg)')
    plt.savefig(plotname+'_comparison_cphase.pdf',bbox_inches='tight',pad_inches=0.)
#plt.figure(figsize=(5,4))
#plt.errorbar(uvdistmax,np.median(cphase,axis=0),np.std(cphase,axis=0)/3.,marker='o',linestyle='',zorder=1,label='median simulated')
#plt.plot(uvdistmax,obs_sim_scan_perfect.cphase,marker='o',linestyle='',zorder=2,label='truth model')
#plt.plot(uvdistmax,obsdata.cphase,marker='o',linestyle='',label='Hops day 3601',zorder=3)
#plt.axis([-0.05e10,0.85e10,-0.1,0.6])
