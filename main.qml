import QtQuick
import Qt5Compat.GraphicalEffects

Item {
    DirectionalBlur {
        id: blurEffect
        anchors.fill: imageItem
        source: imageItem
        angle: 90
        length: 32
        samples: 24
    }

    Image {
        id: blurredImage
        anchors.fill: parent
        source: blurEffect
    }
}