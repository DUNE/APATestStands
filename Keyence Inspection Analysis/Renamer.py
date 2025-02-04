import os
import openpyxl
import tkinter as tk
from tkinter import filedialog, messagebox, ttk
from tkcalendar import Calendar
from datetime import datetime

def rename_excel_files(process_directory, board_type, selected_date):
    # Ensure the directory exists
    if not os.path.exists(process_directory):
        messagebox.showerror("Error", "The process directory does not exist. Please check the path.")
        return

    # Rename each Excel file in the directory
    for filename in os.listdir(process_directory):
        if filename.endswith(".xlsx"):
            original_file_path = os.path.join(process_directory, filename)
            
            # Retrieve the serial number from the Excel file (cell I18)
            serial_number = retrieve_serial_number(original_file_path)
            
            # Generate the new file name
            file_name_parts = filename.split("_")
            new_filename = f"{selected_date}_{board_type}{serial_number}_{file_name_parts[2]}"
            
            # Ensure the new filename doesn't already end with '.xlsx'
            if not new_filename.endswith(".xlsx"):
                new_filename += ".xlsx"
            
            new_file_path = os.path.join(process_directory, new_filename)
            
            # Rename the file
            os.rename(original_file_path, new_file_path)

    messagebox.showinfo("Success", "Files renamed successfully!")

def retrieve_serial_number(file_path):
    # Load the workbook and select the active worksheet
    wb = openpyxl.load_workbook(file_path)
    ws = wb.active
    
    # Retrieve the serial number from cell I18
    serial_number = ws['I18'].value
    
    # Ensure the serial number is in a string format
    serial_number = str(serial_number)
    
    # Close the workbook
    wb.close()
    
    return serial_number

def select_process_directory():
    directory = filedialog.askdirectory()
    process_directory_entry.delete(0, tk.END)
    process_directory_entry.insert(0, directory)

def select_date():
    # Open a calendar to select a date
    def grab_date():
        date_var.set(cal.selection_get().strftime('%Y%m%d'))
        top.destroy()

    top = tk.Toplevel(root)
    top.title("Select Date")
    cal = Calendar(top, selectmode="day", date_pattern='y-mm-dd')
    cal.pack(pady=20)

    tk.Button(top, text="OK", command=grab_date).pack(pady=20)

def start_processing():
    process_directory = process_directory_entry.get()
    board_type = board_type_var.get()
    selected_date = date_var.get()

    if not board_type:
        messagebox.showerror("Error", "Please select a board type.")
        return
    if not selected_date:
        messagebox.showerror("Error", "Please select a date.")
        return

    rename_excel_files(process_directory, board_type, selected_date)

# Create the main window
root = tk.Tk()
root.title("Excel File Renamer")

# Create and place the widgets
tk.Label(root, text="Select Process Directory:").grid(row=0, column=0, padx=10, pady=10)
process_directory_entry = tk.Entry(root, width=50)
process_directory_entry.grid(row=0, column=1, padx=10, pady=10)
tk.Button(root, text="Browse", command=select_process_directory).grid(row=0, column=2, padx=10, pady=10)

# Board type dropdown
tk.Label(root, text="Select Board Type:").grid(row=1, column=0, padx=10, pady=10)
board_type_var = tk.StringVar()
board_type_dropdown = ttk.Combobox(root, textvariable=board_type_var)
board_type_dropdown['values'] = ("GBias", "Adapt", "AdaptBK", "CR", "CRBK")
board_type_dropdown.grid(row=1, column=1, padx=10, pady=10)
board_type_dropdown.current(0)  # Set default value

# Date selection button with calendar
tk.Label(root, text="Select Date:").grid(row=2, column=0, padx=10, pady=10)
date_var = tk.StringVar(value=datetime.now().strftime('%Y%m%d'))
tk.Entry(root, textvariable=date_var, width=20).grid(row=2, column=1, padx=10, pady=10)
tk.Button(root, text="Select Date", command=select_date).grid(row=2, column=2, padx=10, pady=10)

tk.Button(root, text="Start Renaming", command=start_processing).grid(row=3, column=1, pady=20)

# Run the GUI main loop
root.mainloop()
