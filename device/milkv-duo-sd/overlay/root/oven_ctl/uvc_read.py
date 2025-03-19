import subprocess
import base64
import os
import glob

def capture_and_encode():
    return_data = "ERROR"
    os.chdir("camera")
    # 运行 uvcgrab，并阻塞直到其结束
    try:
        subprocess.run(["./uvcgrab", "/dev/video0"], stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL, check=True)

        # 查找最新生成的图片
        image_files = sorted(glob.glob("*.jpg"), key=os.path.getctime, reverse=True)
        if not image_files:
            raise FileNotFoundError("No image file found after running uvcgrab")

        image_path = image_files[0]

        # 读取图片并转换为 Base64
        with open(image_path, "rb") as image_file:
            return_data = base64.b64encode(image_file.read()).decode("utf-8")

        # 删除图片文件
        os.remove(image_path)
    finally:
        os.chdir("..")

    return return_data

if __name__ == "__main__":
    try:
        base64_image = capture_and_encode()
        print(base64_image)
    except Exception as e:
        print(f"Error: {e}")
