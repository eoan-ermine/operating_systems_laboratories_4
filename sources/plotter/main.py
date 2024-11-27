import os
import re
import matplotlib.pyplot as plt
import pathlib
import typer

PATTERN = R"([^ ]{16})\s+([^ ]+)\s+([^ ]+)\s+([^ ]+)\s+([^ ]+)\s+([^ \n]+)"


def main(plots_path: pathlib.Path, dumps_path: pathlib.Path = "/usr/share/operating_systems_laboratories_4/dumps/"):
	addresses = []
	sizes = []
	for filename in os.listdir(dumps_path):
	    path = os.path.join(dumps_path, filename)
	    f = open(path, "r")
	    text = "".join(f.readlines()).replace("[ ", "").replace("] ", "")
	    matches = re.findall(PATTERN, text)
	    for match in matches[:-1]:
	        addresses.append(match[0])
	        sizes.append(int(match[1]))

	    fig, ax = plt.subplots()
	    ax.bar(addresses, sizes)
	    plt.savefig(f"{filename}.png")


if __name__ == "__main__":
	typer.run(main)