# Define variables
EXECUTABLE = cavity3d
STRONG_SCALING_FILE = strong_scaling.txt
WEAK_SCALING_FILE = weak_scaling.txt
PLOT_SCRIPT = plotting.py
REQUIREMENTS_FILE = requirements.txt

# Define targets and dependencies
all: plot

# Step 1: Compile
$(EXECUTABLE): Makefile
	make clean; make

# Step 2: Execute
$(STRONG_SCALING_FILE) $(WEAK_SCALING_FILE): $(EXECUTABLE) scaling.sh
	bash scaling.sh

# Step 3: Install dependencies and plot
plot: $(STRONG_SCALING_FILE) $(WEAK_SCALING_FILE) $(PLOT_SCRIPT) $(REQUIREMENTS_FILE)
	pip install -r $(REQUIREMENTS_FILE)
	python3 $(PLOT_SCRIPT)

# Clean target
clean:
	rm -f $(EXECUTABLE) $(STRONG_SCALING_FILE) $(WEAK_SCALING_FILE)

.PHONY: all plot clean
