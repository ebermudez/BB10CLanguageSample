// Tabbed Pane project template
import bb.cascades 1.0

TabbedPane {
    objectName: "panelTabs"
    showTabsOnActionBar: true

    Tab {
        title: qsTr("Inicio")
        objectName: "Home"
        imageSource: "asset:///images/icon_home.png"
        Noticiero {
        }
    }

    Tab {
        title: qsTr("Buscar")
        objectName: "Buscar"
        imageSource: "asset:///images/icon_buscar.png"
        BuscarScreen {
        }

        onTriggered: {
            mainApp.analyticsEvents("Buscar");
        }
    }

    Tab {
        objectName: "TabOnlinePress"
        title: "Edición Impresa"
        imageSource: "asset:///images/icon_quienes.png"

        EdicionImpresa {
        }

        onTriggered: {
            //Esta instrucción comienza el cambio de una versión a otra
            //y manda a cambiar los tabs
            mainApp.analyticsEvents("Edición Impresa");
        }
    }

    Tab {
        title: qsTr("Fotos")
        imageSource: "asset:///images/icon_fotos.png"
        Fotos {
        }
        onTriggered: {
            mainApp.analyticsEvents("Fotos");
        }
    }

    Tab {
        title: qsTr("Videos")
        imageSource: "asset:///images/icon_videos.png"
        VideoGallery {
        }
        onTriggered: {
            mainApp.analyticsEvents("Videos");
        }
    }

    Tab {
        title: qsTr("Indicadores")
        imageSource: "asset:///images/icono_indicadores.png"
        Indicadores {
        }
        onTriggered: {
            mainApp.analyticsEvents("Indicadores");
        }
    }

    Tab {
        title: qsTr("Favoritos")
        imageSource: "asset:///images/icon_favoritos.png"
        Favoritos {
            id: favsScreen
            objectName: "favsScreen"
        }
        onTriggered: {
            mainApp.loadFavs();
            mainApp.analyticsEvents("Favoritos");
        }
    }

    // Adding the top application menu
    Menu.definition: MenuDefinition {

        actions: [
            ActionItem {
                title: "Acerca De"
                imageSource: "asset:///images/companyIcon.png"
                onTriggered: {
                    openAcercade();

                }
            },

            ActionItem {
                title: "Contacto"
                imageSource: "asset:///images/icon_contacto.png"
                onTriggered: {
                    openContacto();

                }
            },

            ActionItem {
                title: "QuienesSomos"

                imageSource: "asset:///images/icon_quienes.png"
                onTriggered: {
                    openQuienes();

                }
            },
            
            ActionItem {
                title: "Alertas"

                imageSource: "asset:///images/alertas.png"
                onTriggered: {
                    openAlertas();
                    mainApp.analyticsEvents("Alertas");

                }
            }

        ]

    }

    function openAcercade() {
        acercadeSheet.open();
    }

    function closeAcercade() {
        acercadeSheet.close();
    }

    function openContacto() {
        contactoSheet.open();
    }

    function closeContacto() {
        contactoSheet.close();
    }

    function openQuienes() {
        quienesSheet.open();
    }

    function closeQuienes() {
        quienesSheet.close();
    }
    
    function openAlertas() {
        alertasSheet.open();
    }
    
    function closeAlertas() {
        alertasSheet.close();
    }

    onCreationCompleted: {
        // enable layout to adapt to the device rotation
        OrientationSupport.supportedDisplayOrientation = SupportedDisplayOrientation.DisplayPortrait;
        console.log("\n\nTabbedPane - onCreationCompleted " + "" + "\n\n");

        Application.cover = appCover.createObject();
    }

    onActiveTabChanged: {
        mainApp.CargarDatosSecciones();
    }

    attachedObjects: [
        ComponentDefinition {
            id: appCover
            source: "AppCover.qml"
        },

        Sheet {
            id: acercadeSheet
            AcercaDe {

            }
        },

        Sheet {
            id: contactoSheet
            Contacto {

            }
        },

        Sheet {
            id: quienesSheet
            QuienesSomos {

            }
        },
        
        Sheet {
            id: alertasSheet
            objectName: "alertasSheet"
            PushMain {
            
            }
        }
    ]

}
