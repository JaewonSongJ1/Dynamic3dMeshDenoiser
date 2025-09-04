# UI/dynamic3dmesh_denoiser_ui.py
# Dynamic3DMesh Denoiser - Standalone UI (Alembic Denoise tab only)
# PyQt5 기반. 배포용 EXE(BilateralMeshDenoiser.exe / TemporalMeshDenoiser.exe) 실행 래퍼.

import os, sys, subprocess
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QGridLayout, QHBoxLayout,
    QLabel, QLineEdit, QPushButton, QFileDialog, QTextEdit, QProgressBar,
    QSpinBox, QDoubleSpinBox, QRadioButton, QButtonGroup, QGroupBox, QCheckBox,
    QComboBox, QMessageBox
)
from PyQt5.QtCore import QThread, pyqtSignal, Qt

import traceback
def install_excepthook():
    def _hook(exc_type, exc, tb):
        msg = "".join(traceback.format_exception(exc_type, exc, tb))
        try:
            QMessageBox.critical(None, "Unhandled Exception", msg)
        finally:
            # 콘솔 없는 빌드에서도 메시지 확인 후 종료
            os._exit(1)
    sys.excepthook = _hook

# === 공용 유틸 ===

def resource_path(rel_path: str) -> str:
    """PyInstaller/Nuitka 빌드 시에도 동작하도록 리소스 경로 해석."""
    base = getattr(sys, "_MEIPASS", None) or os.path.abspath(os.path.dirname(__file__))
    return os.path.join(base, rel_path)

# === 실행 파일 탐색 유틸 (교체) ===
import os, sys

def find_deploy_dir() -> str:
    """
    실행 파일(.exe) 위치 규칙:
    1) D3MD_BIN 환경변수로 지정된 경로가 있으면 최우선
    2) PyInstaller로 빌드된 UI EXE가 있는 폴더(= deploy)  [sys.frozen == True]
    3) 소스 실행 시: <repo_root>/deploy  [UI/.. 상위로 올라감]
    """
    env = os.environ.get("D3MD_BIN")
    if env and os.path.isdir(env):
        return os.path.abspath(env)

    if getattr(sys, "frozen", False):
        # 빌드된 UI exe가 위치한 폴더 (deploy)
        return os.path.abspath(os.path.dirname(sys.executable))

    # source 실행 시: UI/../deploy
    ui_dir = os.path.abspath(os.path.dirname(__file__))
    repo_root = os.path.dirname(ui_dir)
    return os.path.join(repo_root, "deploy")

def find_denoiser_exe(exe_name: str) -> str:
    """
    BilateralMeshDenoiser.exe / TemporalMeshDenoiser.exe 경로를 resolve.
    deploy 폴더 또는 PATH 둘 중 먼저 발견되는 것을 사용.
    """
    cand = [
        os.path.join(find_deploy_dir(), exe_name),  # deploy/ 내
        exe_name,                                   # PATH
    ]
    for p in cand:
        if os.path.exists(p):
            return p
    return exe_name

class WorkerThread(QThread):
    finished = pyqtSignal()
    error = pyqtSignal(str)
    output = pyqtSignal(str)
    def __init__(self, command: str):
        super().__init__()
        self.command = command
    def run(self):
        try:
            env = os.environ.copy()
            env["PYTHONIOENCODING"] = "utf-8"
            env["PYTHONLEGACYWINDOWSSTDIO"] = "1"
            # shell=True로 표준출력/에러 스트림 텍스트 파이프
            p = subprocess.Popen(
                self.command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                text=True, shell=True, env=env, encoding="utf-8", errors="replace"
            )
            while True:
                line = p.stdout.readline()
                if line == "" and p.poll() is not None:
                    break
                if line:
                    self.output.emit(line.rstrip("\n"))
            rc = p.poll()
            if rc != 0:
                self.error.emit(f"Process failed with return code {rc}: {p.stderr.read()}")
            else:
                self.finished.emit()
        except Exception as e:
            self.error.emit(f"Error running command: {e}")

# === Alembic Denoise 탭 ===
# (구성/옵션/프리셋은 기존 UI의 AlembicDenoiseTab를 반영) :contentReference[oaicite:6]{index=6}
class AlembicDenoiseTab(QWidget):
    def __init__(self):
        super().__init__()
        self.init_ui()

    def init_ui(self):
        layout = QVBoxLayout()

        # 알고리즘 선택
        algo_group = QGroupBox("Denoising Algorithm")
        v = QVBoxLayout()
        self.bilateral_radio = QRadioButton("Bilateral Mesh Denoiser (Production Quality)")
        self.temporal_radio = QRadioButton("Temporal Mesh Denoiser (Fast Preview)")
        self.bilateral_radio.setChecked(True)
        self.algo_buttons = QButtonGroup()
        self.algo_buttons.addButton(self.bilateral_radio)
        self.algo_buttons.addButton(self.temporal_radio)
        self.bilateral_radio.toggled.connect(self._update_advanced_visibility)
        self.temporal_radio.toggled.connect(self._update_advanced_visibility)
        v.addWidget(self.bilateral_radio); v.addWidget(self.temporal_radio)
        algo_group.setLayout(v)
        layout.addWidget(algo_group)

        # IO
        io = QGroupBox("Input/Output")
        g = QGridLayout()
        g.addWidget(QLabel("Input Alembic:"), 0, 0)
        self.in_edit = QLineEdit(); btn_in = QPushButton("Browse…")
        btn_in.clicked.connect(self._pick_input)
        g.addWidget(self.in_edit, 0, 1); g.addWidget(btn_in, 0, 2)

        g.addWidget(QLabel("Output Alembic:"), 1, 0)
        self.out_edit = QLineEdit(); btn_out = QPushButton("Browse…")
        btn_out.clicked.connect(self._pick_output)
        g.addWidget(self.out_edit, 1, 1); g.addWidget(btn_out, 1, 2)
        io.setLayout(g)
        layout.addWidget(io)

        # 기본 설정: 프레임 범위
        basic = QGroupBox("Basic Settings")
        gb = QGridLayout()
        gb.addWidget(QLabel("Start Frame:"), 0, 0)
        self.sf = QSpinBox(); self.sf.setRange(1, 999999); self.sf.setValue(1)
        gb.addWidget(self.sf, 0, 1)
        gb.addWidget(QLabel("End Frame:"), 0, 2)
        self.ef = QSpinBox(); self.ef.setRange(1, 999999); self.ef.setValue(100)
        gb.addWidget(self.ef, 0, 3)
        basic.setLayout(gb)
        layout.addWidget(basic)

        # 고급 옵션 토글
        self.adv_chk = QCheckBox("Show Advanced Options")
        self.adv_chk.toggled.connect(self._toggle_advanced)
        layout.addWidget(self.adv_chk)

        # Temporal 고급
        self.grp_temporal = QGroupBox("Temporal Denoiser Options")
        gt = QGridLayout()
        gt.addWidget(QLabel("Window Size:"), 0, 0)
        self.temporal_window = QSpinBox(); self.temporal_window.setRange(1, 50); self.temporal_window.setValue(7)
        gt.addWidget(self.temporal_window, 0, 1)
        gt.addWidget(QLabel("Weight:"), 1, 0)
        self.weight = QComboBox(); self.weight.addItems(["linear", "gaussian"])
        gt.addWidget(self.weight, 1, 1)
        gt.addWidget(QLabel("Sigma (gaussian):"), 2, 0)
        self.temporal_sigma = QDoubleSpinBox(); self.temporal_sigma.setRange(0.1, 10.0); self.temporal_sigma.setSingleStep(0.1); self.temporal_sigma.setValue(1.0)
        gt.addWidget(self.temporal_sigma, 2, 1)
        self.grp_temporal.setLayout(gt)

        # Bilateral 고급(프리셋 포함)
        self.grp_bilateral = QGroupBox("Bilateral Denoiser Options")
        gbil = QGridLayout()
        gbil.addWidget(QLabel("Window Size:"), 0, 0)
        self.bil_window = QSpinBox(); self.bil_window.setRange(1, 50); self.bil_window.setValue(9)
        gbil.addWidget(self.bil_window, 0, 1)

        gbil.addWidget(QLabel("Sigma Temporal:"), 1, 0)
        self.sigma_temporal = QDoubleSpinBox(); self.sigma_temporal.setRange(0.1,10.0); self.sigma_temporal.setSingleStep(0.1); self.sigma_temporal.setValue(2.5)
        gbil.addWidget(self.sigma_temporal, 1, 1)

        gbil.addWidget(QLabel("Sigma Spatial:"), 2, 0)
        self.sigma_spatial = QDoubleSpinBox(); self.sigma_spatial.setDecimals(3); self.sigma_spatial.setRange(0.01,1.0); self.sigma_spatial.setSingleStep(0.01); self.sigma_spatial.setValue(0.15)
        gbil.addWidget(self.sigma_spatial, 2, 1)

        row = 3
        h = QHBoxLayout()
        btn_subtle = QPushButton("Subtle"); btn_mid = QPushButton("Medium"); btn_strong = QPushButton("Strong")
        btn_subtle.clicked.connect(lambda: self._apply_bilateral_preset("subtle"))
        btn_mid.clicked.connect(lambda: self._apply_bilateral_preset("medium"))
        btn_strong.clicked.connect(lambda: self._apply_bilateral_preset("strong"))
        h.addWidget(QLabel("Presets:")); h.addWidget(btn_subtle); h.addWidget(btn_mid); h.addWidget(btn_strong); h.addStretch(1)
        gbil.addLayout(h, row, 0, 1, 2)
        self.grp_bilateral.setLayout(gbil)

        # 고급 그룹 컨테이너
        self.adv_container = QWidget(); adv_layout = QVBoxLayout(self.adv_container)
        adv_layout.addWidget(self.grp_temporal)
        adv_layout.addWidget(self.grp_bilateral)
        self.adv_container.setVisible(False)
        layout.addWidget(self.adv_container)

        # 실행
        self.btn_run = QPushButton("Process Denoising")
        self.btn_run.clicked.connect(self._run)
        layout.addWidget(self.btn_run)

        layout.addStretch()
        self.setLayout(layout)
        self._update_advanced_visibility()

    def _toggle_advanced(self, on: bool):
        self.adv_container.setVisible(on)

    def _update_advanced_visibility(self):
        if self.bilateral_radio.isChecked():
            self.grp_bilateral.setVisible(True)
            self.grp_temporal.setVisible(False)
        else:
            self.grp_bilateral.setVisible(False)
            self.grp_temporal.setVisible(True)

    def _apply_bilateral_preset(self, preset: str):
        # 기존 UI 프리셋과 동일한 값. :contentReference[oaicite:7]{index=7}
        if preset == "subtle":
            self.bil_window.setValue(7); self.sigma_temporal.setValue(1.5); self.sigma_spatial.setValue(0.08)
        elif preset == "medium":
            self.bil_window.setValue(9); self.sigma_temporal.setValue(2.5); self.sigma_spatial.setValue(0.15)
        elif preset == "strong":
            self.bil_window.setValue(15); self.sigma_temporal.setValue(5.0); self.sigma_spatial.setValue(0.35)

    def _pick_input(self):
        p, _ = QFileDialog.getOpenFileName(self, "Select Input Alembic", "", "Alembic (*.abc)")
        if p: self.in_edit.setText(p)

    def _pick_output(self):
        p, _ = QFileDialog.getSaveFileName(self, "Save Output Alembic", "", "Alembic (*.abc)")
        if p: self.out_edit.setText(p)

    def _run(self):
        in_f = self.in_edit.text().strip()
        out_f = self.out_edit.text().strip()
        sf = self.sf.value(); ef = self.ef.value()
        if not in_f or not out_f:
            QMessageBox.warning(self, "Warning", "Input/Output 경로를 지정하세요.")
            return
        if ef < sf:
            QMessageBox.warning(self, "Warning", "End Frame이 Start Frame보다 작을 수 없습니다.")
            return

        if self.bilateral_radio.isChecked():
            exe = find_denoiser_exe("BilateralMeshDenoiser.exe")
            cmd = f'"{exe}" "{in_f}" "{out_f}" --maya-range {sf} {ef}'
            if self.adv_chk.isChecked():
                cmd += f' --window {self.bil_window.value()} --sigma-temporal {self.sigma_temporal.value()} --sigma-spatial {self.sigma_spatial.value()}'
        else:
            exe = find_denoiser_exe("TemporalMeshDenoiser.exe")
            cmd = f'"{exe}" "{in_f}" "{out_f}" --maya-range {sf} {ef}'
            if self.adv_chk.isChecked():
                cmd += f' --window {self.temporal_window.value()} --weight {self.weight.currentText()}'
                if self.weight.currentText() == "gaussian":
                    cmd += f' --sigma {self.temporal_sigma.value()}'

        # Windows UTF-8 콘솔 대응
        if os.name == "nt":
            cmd = f'chcp 65001 >nul && {cmd}'

        mw = self.window()  # 최상위 QMainWindow
        if hasattr(mw, "execute_command"):
            mw.execute_command(cmd, "Alembic Denoising")
        else:
            QMessageBox.critical(self, "Error", "Main window not found (execute_command).")

# === 메인 윈도우 ===
class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Dynamic3DMesh Denoiser - Alembic")
        self.setGeometry(100, 100, 820, 560)
        w = QWidget(); v = QVBoxLayout(w)

        self.tab = AlembicDenoiseTab()
        v.addWidget(self.tab)

        self.progress = QProgressBar(); self.progress.setVisible(False)
        v.addWidget(self.progress)

        self.log = QTextEdit(); self.log.setReadOnly(True); self.log.setMaximumHeight(180)
        v.addWidget(self.log)

        self.setCentralWidget(w)
        self.worker = None

    # 기존 UI와 동일한 실행 파이프. :contentReference[oaicite:8]{index=8}
    def execute_command(self, command: str, name: str):
        if self.worker and self.worker.isRunning():
            QMessageBox.warning(self, "Warning", "다른 작업이 실행 중입니다.")
            return
        self.log.append(f"\n=== Starting {name} ===")
        self.log.append(f"Command: {command}")
        self.progress.setVisible(True); self.progress.setRange(0, 0)
        self.worker = WorkerThread(command)
        self.worker.finished.connect(self._on_done)
        self.worker.error.connect(self._on_err)
        self.worker.output.connect(self._on_out)
        self.worker.start()

    def _on_done(self):
        self.progress.setVisible(False)
        self.log.append("=== Process completed successfully ===\n")
        QMessageBox.information(self, "Success", "Completed.")

    def _on_err(self, msg: str):
        self.progress.setVisible(False)
        self.log.append(f"ERROR: {msg}")
        self.log.append("=== Process failed ===\n")
        QMessageBox.critical(self, "Error", msg)

    def _on_out(self, line: str):
        self.log.append(line)
        self.log.ensureCursorVisible()

def main():
    install_excepthook
    app = QApplication(sys.argv)
    win = MainWindow(); win.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
