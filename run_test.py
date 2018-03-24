import os,sys,commands

cfg = "inter_tool_test.cfg"

data_folder = "/home/twongjirad/working/data/larbys/mcc8v4/cocktailtest/cocktail_run1_subrun300"

def genFileDict( run, subrun ):
    fid = run*10000+subrun
    f = {"ssnet":data_folder+"/ssnetout-larcv-Run%06d-SubRun%06d.root"%(run,subrun),
         "opreco":data_folder+"/opreco-Run%06d-SubRun%06d.root"%(run,subrun),
         "reco2d":data_folder+"/reco2d-Run%06d-SubRun%06d.root"%(run,subrun),
         "supera":data_folder+"/supera-Run%06d-SubRun%06d.root"%(run,subrun),
         "mcinfo":data_folder+"/mcinfo-Run%06d-SubRun%06d.root"%(run,subrun),         
         "shower":data_folder+"/shower_reco_out_%d.root"%(fid),
         "pgraph":data_folder+"/vertexout_filter_nue_ana_tree_%d.root"%(fid),
         "tracker":data_folder+"/tracker_reco_%d.root"%(fid) }
    return f

fdict = genFileDict(1,300)


args = "%s %s %s"%(fdict["ssnet"],fdict["mcinfo"],"baka_larcv.root")

print args
os.system("./bin/donkey_gen %s"%(args))




