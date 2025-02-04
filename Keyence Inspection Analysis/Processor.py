import os
import pandas as pd
import tkinter as tk
from tkinter import filedialog, messagebox

# The association between measurement IDs and names for "Adapt" boards
adapt_name_to_id_mapping = {
    "Board Length": 1,
    "Board Width": 2,
    "Diameter Hole 1": 3,
    "Diameter Hole 2": 4,
    "Diameter Hole 3": 5,
    "Diameter Hole 4": 6,
    "Diameter Hole 5": 7,
    "Diameter Hole 6": 8,
    "Diameter Hole 7": 9,
    "Diameter Hole 8": 10,
    "Hole 2 X": 11,
    "Hole 3 X": 12,
    "Hole 4 X": 13,
    "Hole 5 X": 14,
    "Hole 6 X": 15,
    "Hole 7 X": 16,
    "Hole 8 X": 17,
    "Diameter Mill Hole 1": 18,
    "Diameter Mill Hole 2": 19,
    "Mill Hole 1 X": 20,
    "Mill Hole 2 X": 21,
    "Mill 1 X": 22,
    "Mill 1 Y": 23,
    "Mill 2 X": 24,
    "Mill 2 Y": 25,
    "Hole 5 Y": 26,
    "Hole 6 Y": 27,
    "Hole 7 Y": 28,
    "Hole 8 Y": 29,
    "Diameter J1": 30,
    "Diameter J2": 31,
    "Pin 1 J1 X": 32,
    "Pin 1 J1 Y": 33,
    "Pin 1 J2 X": 34,
    "Pin 1 J2 Y": 35,
    "Diameter P1": 36,
    "Diameter P2": 37,
    "Pin 1 P1 X": 38,
    "Pin 1 P1 Y": 39,
    "Pin 1 P2 X": 40,
    "Pin 1 P2 Y": 41,
    "Mill Pad 1 Depth": 42,
    "Mill Pad 2 Depth": 43,
    "Board Thickness": 44
}

# Function to assign IDs using the provided mapping for "Adapt" boards based on measurement names
def assign_fixed_ids_based_on_names(df, mapping):
    measurement_names_5 = df.iloc[:, 5].values  # Assuming most measurement names are in column 5

    # Prepare a list to hold assigned IDs
    assigned_ids = []
    assigned_names = []

    # Process names from column 5 (main column for measurements)
    for name in measurement_names_5[:-1]:  # Exclude the last row
        assigned_id = mapping.get(name, None)  # Retrieve the ID from the mapping
        assigned_ids.append(assigned_id)
        assigned_names.append(name)

    # For the last row, assume "Board Thickness"
    assigned_ids.append(44)  # Assign ID 44
    assigned_names.append("Board Thickness")  # Hardcode the name

    # Assign the IDs and names to the appropriate columns (ignoring the IDs from the file)
    df.iloc[:-1, 1] = assigned_ids[:-1]  # Assign all but the last row
    df.iloc[-1, 1] = 44  # Assign ID 44 to the last row
    df.iloc[-1, 5] = "Board Thickness"  # Hardcode the name to the last row
    
    return df

# Function to truncate the filename based on the selected board type
def truncate_filename(original_filename, board_type):
    if board_type in original_filename:
        return f"{board_type}Data" + original_filename.split(board_type)[1].split("_")[0] + ".txt"
    return os.path.splitext(original_filename)[0] + ".txt"

# Function to extract the relevant data based on board type and assign the fixed IDs for "Adapt"
def extract_data(df, board_type):
    if board_type == "Adapt":
        # Assign fixed measurement IDs using the provided mapping
        df = assign_fixed_ids_based_on_names(df, adapt_name_to_id_mapping)
        
        # Now continue the extraction process as in your original code
        measurement_name_1 = df.iloc[31:72, 5].values  # F32-F72
        measurement_name_2 = df.iloc[72:75, 3].values  # D73-D75 (excluding Board Thickness)
        measurement_result = df.iloc[31:75, 8].values  # I32-I75
        design_value = df.iloc[31:75, 10].values  # K32-K75
        upper_tolerance = df.iloc[31:75, 11].values  # L32-L75
        lower_tolerance = df.iloc[31:75, 12].values  # M32-M75

        measurement_name_combined = list(measurement_name_1) + list(measurement_name_2)
        min_length = min(len(measurement_result), len(measurement_name_combined), len(design_value), len(upper_tolerance), len(lower_tolerance))

        return pd.DataFrame({
            "Measurement ID": df.iloc[31:75, 1].values[:min_length],  # Already assigned IDs
            "Measurement Name": measurement_name_combined[:min_length],
            "Measurement Result": measurement_result[:min_length],
            "Design Value": design_value[:min_length],
            "Upper Tolerance": upper_tolerance[:min_length],
            "Lower Tolerance": lower_tolerance[:min_length]
        })
    elif board_type == "GBias":
        return pd.DataFrame({
            "No.": df.iloc[31:60, 1].values,
            "Measurement Result": df.iloc[31:60, 5].values,
            "Design Value": df.iloc[31:60, 8].values,
            "Upper Tolerance": df.iloc[31:60, 10].values,
            "Lower Tolerance": df.iloc[31:60, 11].values
        })
    elif board_type == "AdaptBK":
        return pd.DataFrame({
            "No.": df.iloc[31:39, 1].values,
            "Element1": df.iloc[31:39, 3].values,
            "Measurement Result": df.iloc[31:39, 8].values,
            "Design Value": df.iloc[31:39, 10].values,
            "Upper Tolerance": df.iloc[31:39, 11].values,
            "Lower Tolerance": df.iloc[31:39, 12].values
        })
    elif board_type == "CR":
        return pd.DataFrame({
            "No.": df.iloc[31:65, 1].values,
            "Measurement Result": df.iloc[31:65, 5].values,
            "Design Value": df.iloc[31:65, 8].values,
            "Upper Tolerance": df.iloc[31:65, 10].values,
            "Lower Tolerance": df.iloc[31:65, 11].values,
            "Additional Value": df.iloc[31:65, 12].values
        })
    elif board_type == "CRBK":
        return pd.DataFrame({
            "No.": df.iloc[31:47, 1].values,
            "Element1": df.iloc[31:47, 5].values,
            "Measurement Result": df.iloc[31:47, 8].values,
            "Design Value": df.iloc[31:47, 10].values,
            "Upper Tolerance": df.iloc[31:47, 11].values,
            "Lower Tolerance": df.iloc[31:47, 12].values
        })
    else:
        return pd.DataFrame()  # Return an empty dataframe if the board type doesn't match

# Function to get the target directory based on the board type
def get_target_directory(board_type):
    base_directory = r"C:\Users\kal\OneDrive\Desktop\Analysis Data" #####YOUR DIRECTORY HERE######
    target_directory = os.path.join(base_directory, board_type)
    os.makedirs(target_directory, exist_ok=True)
    return target_directory

def process_files():
    source_directory = source_directory_var.get()
    if not source_directory:
        messagebox.showwarning("No Source Directory Selected", "Please select a source directory.")
        return

    board_type = board_type_var.get()
    if board_type == "Select Board Type" or not board_type:
        messagebox.showwarning("No Board Type Selected", "Please select a board type.")
        return

    target_directory = get_target_directory(board_type)

    for filename in os.listdir(source_directory):
        if filename.endswith(".xlsx") or filename.endswith(".xls"):
            file_path = os.path.join(source_directory, filename)
            try:
                # Load the sheet from the file
                df = pd.read_excel(file_path, sheet_name='Inspection result', header=None)
                
                # Extract the data based on the board type
                df_selected = extract_data(df, board_type)
                
                if df_selected.empty:
                    messagebox.showwarning("No Data Extracted", f"No data extracted for board type {board_type} from file {filename}.")
                    continue

                # Truncate the filename based on the selected board type
                truncated_filename = truncate_filename(filename, board_type)
                txt_path = os.path.join(target_directory, truncated_filename)

                # Save to a text file without headers
                df_selected.to_csv(txt_path, index=False, header=False, sep=',')

            except Exception as e:
                messagebox.showerror("Error Processing File", f"An error occurred while processing {filename}: {str(e)}")
                continue

    messagebox.showinfo("Process Complete", f"All files have been processed and saved to {target_directory}.")

# Setup GUI
root = tk.Tk()
root.title("Excel to Text File Converter")

# Variables to store directory paths
source_directory_var = tk.StringVar()

# Board type dropdown menu
board_type_var = tk.StringVar(root)
board_type_var.set("Select Board Type")  # default value
board_types = ["GBias", "Adapt", "AdaptBK", "CR", "CRBK"]
board_type_menu = tk.OptionMenu(root, board_type_var, *board_types)
board_type_menu.pack(pady=10)

# Button to select source directory
source_button = tk.Button(root, text="Select Source Directory", command=lambda: source_directory_var.set(filedialog.askdirectory(title="Select Source Directory")))
source_button.pack(pady=5)

# Display selected source directory
source_label = tk.Label(root, textvariable=source_directory_var)
source_label.pack(pady=5)

# Button to start processing the files
process_button = tk.Button(root, text="Start File Processing", command=process_files)
process_button.pack(pady=10)

root.mainloop()


