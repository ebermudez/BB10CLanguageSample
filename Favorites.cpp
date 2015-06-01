/**
 * Author: Edgardo Bermúdez
 * This class was modified to be able to be shared online via GitHub. 
 * Favorites.cpp manages the news marked as favorites.
 * 
 * BlackBerry 10 - C++ and QML
 * 
 */

#include "Favorites.hpp"

using namespace bb::cascades;
using namespace bb::system;

Favorites::Favorites()
{

}

Favorites::~Favorites()
{

}

// Methods for the tab favorites
void Favorites::loadFavs(TabbedPane * root)
{
	qDebug("\n\n[Favorites] loadFavs\n");

	this -> root = root;

	DataStoreConfig * store = new DataStoreConfig();
	QString neededWord = "";
	NoticiaHomeWeb * object;

	if(store) {
		QListDataModel<QObject *> * m_model_favoritos;

		m_model_favoritos = new QListDataModel<QObject *>();
		m_model_favoritos -> setParent(this);

		ListView * listViewFavs = root -> findChild<ListView *>("listaFavoritos");

			if(store -> hasKeys()) {
			for(int i = 0; i < store -> keysLength() ; i += store -> AttributesNumber()) {

				QStringList pieces = (store -> getKeys()).at(i).split( "/" );
				neededWord = pieces.value( 0 );

				object = store -> getNoticiaSalvada(neededWord);

				m_model_favoritos -> append (object);
			}

			for(int i = 0; i < m_model_favoritos -> size() ; i++) {
				qobject_cast<NoticiaHomeWeb *>(m_model_favoritos -> value(i)) -> load(true);
			}

			if(listViewFavs) {
				listViewFavs -> setDataModel(m_model_favoritos);
			}

			mostrarEtiqueta("", false);

		} else {
			//Se debe indicar a la pantalla que está vacía la lista de noticias favoritas
			if(listViewFavs) {
				listViewFavs -> resetDataModel();
				listViewFavs -> setDataModel(m_model_favoritos);
			}

			mostrarEtiqueta("No has agregado noticias a tus Favoritos", true);
		}
	} else {
		levantarPopup("Ha ocurrido un error. Intente nuevamente.");
	}

}

void Favorites::mostrarEtiqueta(QString texto, bool mostrar)
{
	qDebug("\n\n[Favorites] mostrarEtiqueta \n");
	qDebug() << "\n\n[Favorites] mostrarEtiqueta " << texto << "\n";

	Label * etiquetaFavs = root -> findChild<Label *>("etiquetaFavs");
	if(etiquetaFavs) {
		qDebug("\n\n[Favorites] etiquetaFavs\n");
		etiquetaFavs -> setText(texto);
		etiquetaFavs -> setVisible(mostrar);
	}
}

// Funciones para agregar y eliminar una noticia

void Favorites::salvarNoticia(NoticiaHomeWeb * noticiaSeleccionada)
{
	qDebug("\n\n[Favorites] salvarNoticia\n");
	DataStoreConfig * store = new DataStoreConfig();

	this -> noticiaSeleccionada = noticiaSeleccionada;
	store -> save(noticiaSeleccionada);

	levantarPopup("Se ha almacenado satisfactoriamente la noticia");
}

void Favorites::eliminarNoticia(NoticiaHomeWeb * noticiaSeleccionada)
{
	qDebug("\n\n[Favorites] eliminarNoticia\n");
	DataStoreConfig * store = new DataStoreConfig();

	this -> noticiaSeleccionada = noticiaSeleccionada;
	store -> removeNews(noticiaSeleccionada -> getId());

	levantarPopup("Se ha eliminado satisfactoriamente la noticia");

	loadFavs(root);
}

void Favorites::eliminarTodas()
{
	qDebug("\n\n[Favorites] eliminarTodas\n");
	DataStoreConfig *store = new DataStoreConfig();

	store -> remove();
	limpiarDataModelFavoritos();

	levantarPopup("Se ha eliminado satisfactoriamente todo el contenido.");
}

void Favorites::limpiarDataModelFavoritos()
{
	qDebug("\n\n[Favorites] limpiarDataModelFavoritos\n");
	ListView * listViewFavs = root -> findChild<ListView *>("listaFavoritos");

	if (listViewFavs) {
		listViewFavs -> resetDataModel();
	}
	mostrarEtiqueta("No has agregado noticias a tus Favoritos", true);
}

// Auxiliares

void Favorites::levantarPopup(QString texto)
{
	qDebug("\n\n[Favorites] levantarPopup\n");
	bb::system::SystemToast * dialog = new bb::system::SystemToast();
	dialog -> setBody(texto);
	dialog -> show();
}
