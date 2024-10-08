#include "mainwindow.h"
#include <Qt3DRender/QMesh>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QOrbitCameraController>
#include <QVector3D>
#include <QQuaternion>
#include <QUrl>
#include <QDir>
#include <QFileInfoList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QJsonValue>
#include <Qt3DRender/qcamera.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DRender/qcameralens.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QCheckBox>
#include <Qt3DRender/qmaterial.h>
#include <Qt3DRender/qsceneloader.h>
#include <Qt3DRender/qpointlight.h>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DCore/qtransform.h>
#include <Qt3DExtras/qforwardrenderer.h>
#include <Qt3DExtras/qt3dwindow.h>
#include <Qt3DExtras/qfirstpersoncameracontroller.h>
#include <QRegularExpression>
// Constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    rotationAngle(0.0f), // Initialize rotation angle
    rotating(false)
{
    setupUi();
    resize(1920, 1080);

    rotationTimer = new QTimer(this);
    connect(rotationTimer, &QTimer::timeout, this, &MainWindow::updateRotation);
}
MainWindow::~MainWindow()
{

}

void MainWindow::updateRotation() {
    rotationAngle += 1.0f;
    float translationAmount = 0.01f;
    for (const auto &child : rootEntity->children()) {
        auto entity = qobject_cast<Qt3DCore::QEntity*>(child);
        if (entity) {
            auto transform = entity->findChild<Qt3DCore::QTransform*>();
            if (transform) {
                // Rotate around the Z-axis for vertical rotation
                QQuaternion rotation = QQuaternion::fromEulerAngles(0.0f, 0.0f, rotationAngle);
                transform->setRotation(rotation);
                // Translate along the X-axis
                QVector3D currentTranslation = transform->translation();
                QVector3D newTranslation = currentTranslation + QVector3D(translationAmount, 0.0f, 0.0f); // Move along X
                transform->setTranslation(newTranslation);
            }
        }
    }
}

void MainWindow::loadObjFiles(const QString& directoryPath, Qt3DCore::QEntity* rootEntity) {
    // Load JSON file containing transformation data
    QFile jsonFile(":/file/resources/dhParameters.json"); // Load JSON file here
    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open JSON file.";
    }

    QByteArray jsonData = jsonFile.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(jsonData));
    QJsonObject jsonObject = doc.object();
    qDebug() << "JSON file loaded successfully with keys:" << jsonObject.keys(); // Debugging JSON keys

    // Create a QDir object for the specified directory
    QDir dir(directoryPath);
    if (!dir.exists()) {
        qWarning() << "Directory does not exist: " << directoryPath;
        return;
    }

    // Set up the filter to find OBJ files
    dir.setNameFilters(QStringList() << "*.obj");
    dir.setFilter(QDir::Files);

    // Get the list of OBJ files
    QFileInfoList fileList = dir.entryInfoList();

    // Iterate through the list of OBJ files
    for (const QFileInfo &fileInfo : fileList) {
        const QString& filePath = fileInfo.absoluteFilePath();
        QString geometryName = fileInfo.baseName(); // Get the base name without the file extension


        // Create an entity to hold the 3D model
        Qt3DCore::QEntity *entity = new Qt3DCore::QEntity(rootEntity);

        // Load the OBJ file
        Qt3DRender::QMesh *mesh = new Qt3DRender::QMesh();
        mesh->setSource(QUrl::fromLocalFile(filePath));  // Ensure this loads the .obj file with .mtl references

        // Apply transformations (translation, rotation)
        Qt3DCore::QTransform *transform = new Qt3DCore::QTransform();


        // Check if the JSON object contains transformation data for this geometry

        if (!jsonObject.isEmpty() && jsonObject.contains(geometryName)) {

            QJsonObject geomData = jsonObject.value(geometryName).toObject();



            // Extract translation and rotation values

            float Tx = geomData.value("Tx").toDouble();

            float Tz = geomData.value("Tz").toDouble();

            float Rx = geomData.value("Rx").toDouble();

            float Rz = geomData.value("Rz").toDouble();

            // Set the translation and rotation values

           // transform->setTranslation(QVector3D(Tx, 0.0f, Tz));

            transform->setRotationX(Rx);

            transform->setRotationZ(Rz);

        } else {

            // Default transformations if no data in JSON


            transform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));

            transform->setRotationX(0.0f);

            transform->setRotationZ(0.0f);



        }

        transform->setScale(0.1f);  // Example scale

        // Create a material to hold the properties from the MTL file
        Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial();

        // Check for the existence of the corresponding MTL file
        QString mtlFilePath = QFileInfo(filePath).absolutePath() + "/" + geometryName + ".obj.mtl";
        QFile mtlFile(mtlFilePath);
        if (mtlFile.exists()) {
            // Load the MTL file
            QColor ambientColor, diffuseColor, specularColor;
            float shininess = 0.0f, transparency = 0.0f;
            int illumModel = 2; // Default illumination model to Phong

            if (parseMtlFile(mtlFilePath, ambientColor, diffuseColor, specularColor, shininess, transparency, illumModel)) {
                material->setAmbient(ambientColor);
                material->setDiffuse(diffuseColor);
                material->setSpecular(specularColor);
                material->setShininess(shininess);

                // Handle transparency by adjusting the alpha channel
                QColor diffuseWithAlpha = diffuseColor;
                diffuseWithAlpha.setAlphaF(1.0f - transparency);  // Adjust alpha based on transparency
                material->setDiffuse(diffuseWithAlpha);

            } else {
                // Set a default material if parsing fails
                material->setAmbient(QColor::fromRgbF(0.2, 0.2, 0.2));
                material->setDiffuse(QColor::fromRgbF(0.498039, 0.498039, 0.498039));
                material->setSpecular(QColor::fromRgbF(1.0, 1.0, 1.0));
                material->setShininess(0.0);
            }
        } else {
            // Set a default material if MTL file does not exist
            material->setAmbient(QColor::fromRgbF(0.2, 0.2, 0.2));
            material->setDiffuse(QColor::fromRgbF(0.498039, 0.498039, 0.498039));
            material->setSpecular(QColor::fromRgbF(1.0, 1.0, 1.0));
            material->setShininess(0.0);
        }

        // Add the components (mesh, transformation, material) to the entity
        entity->addComponent(mesh);
        entity->addComponent(transform);
        entity->addComponent(material);
    }
}

// Sample function to parse the MTL file and return colors
bool MainWindow::parseMtlFile(const QString& mtlFilePath, QColor& ambient, QColor& diffuse, QColor& specular, float& shininess, float& transparency, int& illumModel) {
    QFile mtlFile(mtlFilePath);
    if (!mtlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open MTL file:" << mtlFilePath;
        return false;
    }

    QTextStream in(&mtlFile);
    QString line;

    while (in.readLineInto(&line)) {
        QStringList tokens = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (tokens.isEmpty()) continue;

        if (tokens[0] == "Ka") { // Ambient color
            ambient = QColor::fromRgbF(tokens[1].toFloat(), tokens[2].toFloat(), tokens[3].toFloat());
        } else if (tokens[0] == "Kd") { // Diffuse color
            diffuse = QColor::fromRgbF(tokens[1].toFloat(), tokens[2].toFloat(), tokens[3].toFloat());
        } else if (tokens[0] == "Ks") { // Specular color
            specular = QColor::fromRgbF(tokens[1].toFloat(), tokens[2].toFloat(), tokens[3].toFloat());
        } else if (tokens[0] == "Ns") { // Shininess
            shininess = tokens[1].toFloat();
        } else if (tokens[0] == "Tr") { // Transparency
            transparency = tokens[1].toFloat();
        } else if (tokens[0] == "illum") { // Illumination model
            illumModel = tokens[1].toInt();
        }
    }

    return true; // Successfully parsed the MTL file
}

// Function to create UI controls
void MainWindow::createControls() {
    controlsWidget = new QWidget(this);
    layout = new QVBoxLayout(controlsWidget);

    // Button to reset the camera position
    QPushButton *resetButton = new QPushButton("Reset Camera", this);
    connect(resetButton, &QPushButton::clicked, [this]() {
        // Reset the perspective projection (FOV)
        camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

        // Reset the camera position
        camera->setPosition(QVector3D(0.159012, -504.859, -10.8127));

        // Reset the view center (where the camera is looking)
        camera->setViewCenter(QVector3D(0, 0, 0));

        // Optional: Reset the camera's up vector to ensure proper orientation
        camera->setUpVector(QVector3D(0, 1, 0));
    });

    layout->addWidget(resetButton);
    QSlider *zoomSlider = new QSlider(Qt::Horizontal, this);
    zoomSlider->setRange(1, 100); // Adjusted range for zoom levels
    zoomSlider->setValue(50); // Default zoom (50%)
    connect(zoomSlider, &QSlider::valueChanged, [this](int value) {
        float zoomFactor = 45.0f + (50.0f - value); // Modify FOV based on slider value
        camera->lens()->setPerspectiveProjection(zoomFactor, 16.0f/9.0f, 0.1f, 1000.0f); // Update FOV
    });

    layout->addWidget(zoomSlider);

    controlsWidget->setLayout(layout);
    layout->setSizeConstraint(QLayout::SetFixedSize); // Optional: keep the control size fixed

    // Add button to toggle rotation
    QPushButton *toggleRotationButton = new QPushButton("Toggle Rotation", this);
    connect(toggleRotationButton, &QPushButton::clicked, [this]() {
        static bool rotating = false; // Static variable to retain state across calls
        rotating = !rotating; // Toggle rotation state
        if (rotating) {
            rotationTimer->start(16); // Start rotation
        } else {
            rotationTimer->stop(); // Stop rotation
        }
    });
    layout->addWidget(toggleRotationButton);

    // Add controls to the main window
    setMenuWidget(controlsWidget);
}

// Function to set up the UI and 3D scene
void MainWindow::setupUi() {
    // Create the 3D window
    view = new Qt3DExtras::Qt3DWindow();
    rootEntity = new Qt3DCore::QEntity();
    // Set background color
    view->defaultFrameGraph()->setClearColor(QColor(Qt::darkCyan));

    // Set up the camera
    camera = view->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0.159012, -504.859, -10.8127));
    camera->setViewCenter(QVector3D(0, 0, 0));
    // Set up the light
    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");          // Set light color
    light->setIntensity(1.0f);         // Set light intensity
    lightEntity->addComponent(light);

    // Set light position
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(QVector3D(0.159012, -504.859, -10.8127)); // Position light at the same height as the camera
    lightEntity->addComponent(lightTransform);

    // Create a directional light if you want to light the whole scene uniformly
    Qt3DCore::QEntity *directionalLightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QDirectionalLight *directionalLight = new Qt3DRender::QDirectionalLight(directionalLightEntity);
    directionalLight->setColor("white");
    directionalLight->setIntensity(1.0f);
    directionalLightEntity->addComponent(directionalLight);

    // Set the direction of the directional light
    Qt3DCore::QTransform *directionalLightTransform = new Qt3DCore::QTransform(directionalLightEntity);
    directionalLightTransform->setRotation(QQuaternion::fromEulerAngles(45, 45, 0)); // Adjust as needed
    directionalLightEntity->addComponent(directionalLightTransform);

    // For camera controls
    Qt3DExtras::QOrbitCameraController *camController = new Qt3DExtras::QOrbitCameraController(rootEntity);
    camController->setCamera(camera);

    // Load OBJ files from the resources directory
    QString resourcesDir = ":/file/resources/";
    loadObjFiles(resourcesDir, rootEntity);

    // Set the root entity
    view->setRootEntity(rootEntity);

    // Create a window container
    QWidget *container = QWidget::createWindowContainer(view, this);
    setCentralWidget(container);
    setWindowTitle("3D Image Viewer");
    createControls();
}
