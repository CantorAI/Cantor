import yaml
from pathlib import Path

path = Path(__file__).parent
cfg_file = path.joinpath("test.yaml")
with open(cfg_file, 'r') as file:
    prime_service = yaml.safe_load(file)
    print(prime_service)
print("end")
