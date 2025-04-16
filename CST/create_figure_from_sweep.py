import sys
sys.path.append("venv/lib/python3.13/site-packages")

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Load the original sweep CSV file
df = pd.read_csv("result_sweep.csv", delimiter=";")
df.columns = df.columns.str.strip().str.replace('"', '').str.replace(' ', '_')
df = df.rename(columns={
    '1D_Results\\Es_Solver\\Capacitance_Matrix\\C_potential,_ground': 'C_potential_ground'
})

# Group by geometry and compute ΔF
print("Available columns:", df.columns.tolist())
grouped = df.groupby(['electrode_spacing', 'electrode_width'])

# We'll store delta_F values here
deltaF_rows = []

for (spacing, width), group in grouped:
    if len(group['water_height'].unique()) != 2:
        continue  # skip if both low and high water heights not present
    c_low = group[group['water_height'] == group['water_height'].min()]['C_potential_ground'].values
    c_high = group[group['water_height'] == group['water_height'].max()]['C_potential_ground'].values
    if len(c_low) > 0 and len(c_high) > 0:
        delta_F = c_high[0] - c_low[0]
        deltaF_rows.append({
            'electrode_spacing': spacing,
            'electrode_width': width,
            'delta_F': delta_F
        })

# Convert to DataFrame
deltaF_df = pd.DataFrame(deltaF_rows)
deltaF_df["delta_F"] = deltaF_df["delta_F"].abs()

# Pivot for heatmap
heatmap_data = deltaF_df.pivot(index="electrode_spacing", columns="electrode_width", values="delta_F")

# Plot the heatmap
plt.figure(figsize=(8, 6))
sns.heatmap(heatmap_data, annot=True, fmt=".2e", cmap="viridis", cbar_kws={"label": "ΔF (F)"})
plt.title("Capacitance Change (ΔF) vs Electrode Geometry")
plt.xlabel("Electrode Width (mm)")
plt.ylabel("Electrode Spacing (mm)")
plt.tight_layout()
plt.show()
