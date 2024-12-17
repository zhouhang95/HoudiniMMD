# Get the dialog script of a HDA
import sys
import hou

if len(sys.argv) < 2:
    print("Please provide the path to the HDA")
    print("Usage: hython script_4_get_ds.py your_hda.hda")
    sys.exit(1)

# 从命令行获取 HDA 路径
hda_path = sys.argv[1]

try:
    # 加载 HDA
    hou.hda.installFile(hda_path)
    print(f"Loaded HDA: {hda_path}")
    # 获取 HDA 的所有定义
    hda_definitions = hou.hda.definitionsInFile(hda_path)
    if not hda_definitions:
        raise Exception("No HDA definitions found in file")
    for hda_definition in hda_definitions:
        print(f"HDA Definition: {hda_definition.nodeTypeName()}")
        hda_parm_template_group = hda_definition.parmTemplateGroup()
        hda_dialog_script = hda_parm_template_group.asDialogScript()
        print(hda_dialog_script)
    print("\n")
except Exception as e:
    print(f"Error loading HDA: {e}")
    sys.exit(1)