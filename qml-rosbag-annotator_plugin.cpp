#include "qml-rosbag-annotator_plugin.h"
#include "rosbagannotator.h"
#include "imageitem.h"

#include <qqml.h>

void Qml_Rosbag_AnnotatorPlugin::registerTypes(const char *uri)
{
    // @uri ch.epfl.chili
    qmlRegisterType<RosBagAnnotator>(uri, 1, 0, "RosBagAnnotator");
    qmlRegisterType<ImageItem>(uri, 1, 0, "ImageItem");
}
