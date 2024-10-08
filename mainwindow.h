#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/Qt3DWindow>
#include <QStringList>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <Qt3DRender/QCamera>
#include <QTimer>
QT_BEGIN_NAMESPACE
namespace Qt3DCore {
class QEntity;
}
namespace Qt3DRender {
class QMesh;
}
namespace Qt3DExtras {
class QPhongMaterial;
class QOrbitCameraController;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupUi();
    void createControls();
    void loadObjFiles(const QString& directoryPath, Qt3DCore::QEntity* rootEntity);
    bool parseMtlFile(const QString& mtlFilePath, QColor& ambient, QColor& diffuse, QColor& specular, float& shininess, float& transparency, int& illumModel);
    void updateRotation();
    Qt3DExtras::Qt3DWindow *view;
    Qt3DCore::QEntity *rootEntity;
    Qt3DRender::QCamera *camera;
    QWidget *controlsWidget;
    QVBoxLayout *layout;
    QTimer *rotationTimer;      // Timer for rotation
    float rotationAngle;        // Current rotation angle
    bool rotating;
};

#endif // MAINWINDOW_H
