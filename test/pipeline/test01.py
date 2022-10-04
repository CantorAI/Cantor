import yaml
from pathlib import Path

path = Path(__file__).parent
cfg_file = path.joinpath("pipeline01.yaml")
with open(cfg_file, 'r') as file:
    pipeline_desc = yaml.safe_load(file)
    acts = pipeline_desc["Activities"]
    for act in acts:
        print(act.keys()[0])
    print(pipeline_desc)
print("end")