import os
from PIL import Image
texDir = "."
image = Image.new('RGB', (1, 1))
texNames = [name for name in os.listdir(texDir) if os.path.isdir(os.path.join(texDir,name))]
for tn in texNames:
    image.save(os.path.join(texDir,tn) + ".png", "PNG")