import os
import requests
import platform
import re
from pathlib import Path
import zipfile
import sys

if len(sys.argv) == 1:
    raise RuntimeError(f"Script require argument of directory name for wxWidgets folder")
wx_dir_name = sys.argv[1]

cwd = Path(os.getcwd())

github_token = os.environ.get("GITHUB_TOKEN")
headers = {
    'Accept': 'application/vnd.github+json',
    'Authorization': f'Bearer {github_token}',
    'X-GitHub-Api-Version': '2022-11-28',
}

os_name = platform.system()

if os_name == "Windows":
    wx_name = "wxWidgets-windows"
elif os_name == "Linux":
    wx_name = "wxWidgets-linux"
elif os_name == "Darwin":
    os_name = platform.platform()
    if 'arm' in os_name:
        wx_name = 'wxWidgets-macos-latest'
    else:
        wx_name = 'wxWidgets-macos-[0-9][0-9]-large'
else:
    raise Exception(f"{os_name} operating system not implemented")

response = requests.get('https://api.github.com/repos/NREL/lk/actions/artifacts', headers=headers)
r = response.json()
artifacts = r['artifacts']

matching_artifacts = [art for art in artifacts if re.search(wx_name, art['name'])]
artifact = matching_artifacts[0]

headers = {
    'Accept': 'application/vnd.github+json',
    'Authorization': f'Bearer {github_token}',
    'X-GitHub-Api-Version': '2022-11-28',
}

response = requests.get(
    artifact['archive_download_url'],
    headers=headers,
)

with open(cwd / 'wx.zip', 'wb') as f:
    f.write(response.content)

with zipfile.ZipFile('wx.zip', "r") as zip_ref:
    zip_ref.extractall(cwd / f"{wx_dir_name}")

print(f"Extracted to {str(cwd / wx_dir_name)}")