/**
 * Author: Edgardo Bermúdez
 * This class was modified to be able to be shared online via GitHub. 
 * Main class that controls the main activities in this magazine.
 * 
 * BlackBerry 10 - C++ and QML
 * 
 */

#include "MagazineBB10.hpp"
#include "utilities/Timer.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/device/DisplayInfo>
#include <bb/system/InvokeRequest>
#include <Flurry.h>
#include <cctype>
#include <algorithm>
#include <string>
#include <iostream>
#include <bps/navigator.h>
#include <bps/navigator_invoke.h>

using namespace bb::cascades;

MagazineBB10::MagazineBB10(bb::cascades::Application *app)
: QObject(app)
, m_invokeManager(new InvokeManager(this))
, m_model(new QListDataModel<QObject*>())
, m_model_noticias(new QListDataModel<QObject*>())
, m_model_destac(new QListDataModel<QObject*>())
, m_model_secciones(new QListDataModel<QObject*>())
, m_model_noticias_p(new QListDataModel<QObject*>())
, m_model_noticias_p_2(new QListDataModel<QObject*>())
, m_model_detalle(new QListDataModel<QObject*>())
{
	//Login
	QCoreApplication::setOrganizationName("MyCompanyName");
	QCoreApplication::setOrganizationDomain("mycompany.com");
	QCoreApplication::setApplicationName("My Magazine");

	qmlRegisterType<Timer>("CustomTimer", 1, 0, "Timer");
	qmlRegisterType<Timer>("Multimedia", 1, 0, "Multimedia");
	qmlRegisterType<NoticiaHomeWeb>("MagazineLib", 1, 0, "NoticiaHomeWeb");
	qmlRegisterType<PushApi>();

	// The SceneCover is registered so that it can be used in QML
	qmlRegisterType<SceneCover>("bb.cascades", 1, 0, "SceneCover");

	// Since it is not possible to create an instance of the AbstractCover
	// it is registered as an uncreatable type (necessary for accessing
	// Application.cover).
	qmlRegisterUncreatableType<AbstractCover>("bb.cascades", 1, 0, "AbstractCover",
			"An AbstractCover cannot be created.");

	this->application = app;

	aceptaModoBoth = false;
	modo = 1;
	modo_o_i_secciones = true;
	idSeccImpresa = "";

	// Splash
	QmlDocument *qmlSplash = QmlDocument::create("asset:///splash.qml").parent(this);
	splash = qmlSplash->createRootObject<AbstractPane>();
	application->setScene(splash);

	//Fin splash

	init();

	seccionesFacade = new CiudadFacade();
	setStatusBar("Descargando secciones...");//Splash
	connect(seccionesFacade, SIGNAL(complete()),this, SLOT(cargarSecciones()));
	seccionesFacade->getCity();

	// connect to InvokeManager "invoked" signal to handle incoming push notifications.
	// We will ignore non-push invoke requests.

	connect(this, SIGNAL(setSceneComplete()), SLOT(onSetSceneComplete()));

}

void MagazineBB10::init()
{
	bb::device::DisplayInfo display;
	QDeclarativePropertyMap* pantalla = new QDeclarativePropertyMap;
	pantalla->insert("screenWidth", QVariant(display.pixelSize().width()));
	pantalla->insert("screenHeight", QVariant(display.pixelSize().height()));

	QDeclarativePropertyMap* idSeccion = new QDeclarativePropertyMap;
	idSeccion->insert("id", "");

	NoticiaHomeWeb *noticia2 = new NoticiaHomeWeb();

	share = new Sharing();
	indicadores = new IndicadoresLoader();
	galeriaFotos = new GaleriaFotosLoader();
	galeriaVideos = new GaleriaVideosLoader();
	pushApi = new PushApi(this);
	favorites = new Favorites();
	impresa = new EdicionImpresaLoader();

	tabsPress = new QList<Tab*>();
	tabsOnline = new QList<Tab*>();

	engine = new Engine();

	qmlHome = QmlDocument::create("asset:///mainMagazine.qml");

	qmlHome ->setContextProperty("mainApp", this);
	qmlHome ->setContextProperty("pantalla", pantalla);
	qmlHome ->setContextProperty("modelNewsHomeWeb", m_model_detalle);
	qmlHome ->setContextProperty("noticiaPalQML", noticia2);
	qmlHome ->setContextProperty("modelNewsHome", m_model);
	qmlHome ->setContextProperty("idSeccion", idSeccion);
	qmlHome ->setContextProperty("share", share);
	qmlHome ->setContextProperty("indicadores", indicadores);
	qmlHome ->setContextProperty("galeriaFotos", galeriaFotos);
	qmlHome ->setContextProperty("galeriaVideos", galeriaVideos);
	qmlHome ->setContextProperty("pushAPIHandler", pushApi);
	qmlHome ->setContextProperty("publicidadEngine", engine);

	loginLoader = new LoginLoader(this);
	connect(m_invokeManager, SIGNAL(invoked(const bb::system::InvokeRequest&)),SLOT(onInvoked(const bb::system::InvokeRequest&)));
	connect(loginLoader, SIGNAL(userChanged()),this, SLOT(loginFinished()));
	qmlHome->setContextProperty("loginLoader", loginLoader);
	qmlHome -> setContextProperty("favorites", favorites);
	qmlHome -> setContextProperty("impresa", impresa);

	root = qmlHome->createRootObject<TabbedPane>();

	qmlHomeImpreso = QmlDocument::create("asset:///mainMagazineImpresa.qml");

	qmlHomeImpreso ->setContextProperty("mainApp", this);
	qmlHomeImpreso ->setContextProperty("pantalla", pantalla);
	qmlHomeImpreso ->setContextProperty("modelNewsHomeWeb", m_model_noticias_p_2);
	qmlHomeImpreso ->setContextProperty("noticiaPalQML", noticia2);
	qmlHomeImpreso ->setContextProperty("modelNewsHome", m_model_noticias);
	qmlHomeImpreso ->setContextProperty("idSeccion", idSeccion);
	qmlHomeImpreso ->setContextProperty("share", share);
	qmlHomeImpreso ->setContextProperty("indicadores", indicadores);
	qmlHomeImpreso ->setContextProperty("galeriaFotos", galeriaFotos);
	qmlHomeImpreso ->setContextProperty("galeriaVideos", galeriaVideos);
	qmlHomeImpreso -> setContextProperty("favorites", favorites);
	qmlHomeImpreso ->setContextProperty("publicidadEngine", engine);

	rootImpreso = qmlHomeImpreso->createRootObject<TabbedPane>();
	impreso = rootImpreso;
	online = root;

	qmlDetalleGaleria = QmlDocument::create("asset:///DetalleGaleria.qml");
	qmlDetalleGaleria->setContextProperty("mainApp", this);
	qmlDetalleGaleria->setContextProperty("share", share);
	qmlDetalleGaleria->setContextProperty("loginLoader", loginLoader);

	pagina = qmlDetalleGaleria->createRootObject<Page>();

	qmlDetalleGaleriaVideos = QmlDocument::create("asset:///DetalleGaleriaVideos.qml");
	qmlDetalleGaleriaVideos -> setContextProperty("mainApp", this);
	qmlDetalleGaleriaVideos -> setContextProperty("share", share);
	qmlDetalleGaleriaVideos->setContextProperty("loginLoader", loginLoader);
	qmlDetalleGaleriaVideos -> setContextProperty("favorites", favorites);

	paginaVideos = qmlDetalleGaleriaVideos->createRootObject<Page>();

	//Login persistencia
	fprintf(stderr,"\n MagazineBB10 - Usuario \n");
	fprintf(stderr,(loginLoader->getUser()).toStdString().c_str());

	qmlImpreso = QmlDocument::create("asset:///ListaImpresa.qml");
	qmlImpreso->setContextProperty("mainApp", this);
	qmlImpreso->setContextProperty("share", share);
	qmlImpreso -> setContextProperty("favorites", favorites);
	qmlImpreso->setContextProperty("loginLoader", loginLoader);

	pressPage = qmlImpreso->createRootObject<Page>();

	qmlDetalleImpreso = QmlDocument::create("asset:///NoticiasDetalleSeccionesImpresas.qml");
	qmlDetalleImpreso->setContextProperty("mainApp", this);
	qmlDetalleImpreso->setContextProperty("share", share);
	qmlDetalleImpreso -> setContextProperty("favorites", favorites);
	qmlDetalleImpreso->setContextProperty("loginLoader", loginLoader);
	qmlDetalleImpreso ->setContextProperty("publicidadEngine", engine);


	qmlDetalleImpreso2 = QmlDocument::create("asset:///Galeria.qml");
	qmlDetalleImpreso2 -> setContextProperty("mainApp", this);
	qmlDetalleImpreso2 -> setContextProperty("share", share);
	qmlDetalleImpreso2 -> setContextProperty("favorites", favorites);
	qmlDetalleImpreso2 -> setContextProperty("loginLoader", loginLoader);

	pressSite = qmlDetalleImpreso->createRootObject<Page>();

	qmlSheet = QmlDocument::create("asset:///LoginSheet.qml");
	sheetLogin = qmlSheet->createRootObject<Page>();

	cambiarImpreso();

	engine->descargarPublicidadHome();
	engine->descargarPublicidadDetalle();
	pushApi->initializePushSession();
}

/**
 *  Alterna entre Online e Impreso
 */
void MagazineBB10::cambiarImpreso() {

	if(modo == 1) {
		m_model_detalle = m_model_noticias;

		cambiarOnline();
		modo = 2;
	}else {
		m_model_detalle = m_model_noticias_p_2;

		cambiarPress();
		modo = 1;
	}
}
/** Usado en la barra superior del home que esta oculta
 *  Cambia a edicion online
 */
void MagazineBB10::cambiarOnline()
{

	qDebug() << "\n\n\nEstamos en online \n\n\n";

	ListView* noticias = root->findChild<ListView*>("listaNoticias");
	if(noticias){
		noticias -> setDataModel(m_model);
	}

	ListView* noticias_detalle = Application::instance()->findChild<ListView*>("listaNoticiasDetalle");
	if(noticias_detalle){
		qDebug() << "\n\n\nCambiamos el dataModel a m_model_noticias" << "\n\n\n";
		noticias_detalle -> setDataModel(m_model_noticias);
	}

}

/** Usado en la barra superior del home que esta oculta
 *  Cambia a impreso
 */
void MagazineBB10::cambiarPress() {
	qDebug() << "\n\n\nEstamos en impreso \n\n\n";

	tabsOnline = new QList<Tab *>();

	if(m_model_noticias_p -> isEmpty()){
		setVisibilityOnHome(true);
	}else{
		setVisibilityOnHome(false);
	}

	ListView* noticias = root->findChild<ListView*>("listaNoticias");
	if(noticias){
		noticias -> setDataModel(m_model_noticias_p);
	}

	ListView* noticias_detalle = Application::instance()->findChild<ListView*>("listaNoticiasDetalle");
	if(noticias_detalle){
		noticias_detalle -> setDataModel(m_model_noticias_p_2);
	}
}

/**
 *
 */
void MagazineBB10::getDataModel(){
	if(modo == 1){
		ListView* noticias_detalle = Application::instance()->findChild<ListView*>("listaNoticiasDetalle");
		if(noticias_detalle){
			qDebug() << "\n\n\nCambiamos el dataModel a m_model_noticias" << "\n\n\n";
			noticias_detalle -> setDataModel(m_model_noticias);
		}
	}else{
		ListView* noticias_detalle = Application::instance()->findChild<ListView*>("listaNoticiasDetalle");
		if(noticias_detalle){
			qDebug() << "\n\n\nCambiamos el dataModel a m_model_noticias_p_2" << "\n\n\n";
			noticias_detalle -> setDataModel(m_model_noticias_p_2);
		}
	}
}

void MagazineBB10::insertarTabs() {
	addStaticPressTabs();
	if (tabsPress) {
		for(int i = 0; i < tabsPress -> count() ; i++){
			root -> insert(i, tabsPress -> at(i) );
		}
	}
}

void MagazineBB10::onInvoked(const InvokeRequest &request)
{
	if (request.action().compare("bb.action.OPEN") == 0)
	{
		pushApi->initializePushService();
		qDebug() << "Received open action";
		// Received an invoke request to open an existing push (ie. from a notification in the BlackBerry Hub)
		// The payload from the open invoke is the seqnum for the push in the database
		QCoreApplication::setOrganizationName("MyCompanyName");
		QCoreApplication::setOrganizationDomain("mycompany.com");
		QCoreApplication::setApplicationName("My Magazine");

		QSettings settings;
		settings.setValue("invoke-alertas", request.data().toInt());
	}
}

void MagazineBB10::onSetSceneComplete()
{
	QSettings settings;
	if (settings.contains("invoke-alertas"))
	{
		goAlertas();
		settings.remove("invoke-alertas");
	}
}

//Splash
void MagazineBB10::iniciarTimer()
{
	splashTimer = new QTimer(this);

	connect(splashTimer, SIGNAL(timeout()), this, SLOT(cargarAplicacion()));

	// Set timer properties
	splashTimer->setInterval(3000);
	splashTimer->setSingleShot(true);
	// Start timer
	splashTimer->start();
}

void MagazineBB10::colocarPublicidad(int adserver, int screen){
	
}

bool MagazineBB10::isAceptaModoBoth() const {
	return aceptaModoBoth;
}

void MagazineBB10::setAceptaModoBoth(bool aceptaModoBoth) {
	this->aceptaModoBoth = aceptaModoBoth;
}

void MagazineBB10::cargarAplicacion()
{
	cambioHome();
}

void MagazineBB10::setStatusBar(QString status)
{
	Label * statusLabel = splash->findChild<Label *>("statusBarSplash");
	if (statusLabel)
	{
		statusLabel->setText(status);
	}
}

//Fin splash

/**
 * TODO cargarNoticiasHome
 */
void MagazineBB10::cargarNoticiasHome()
{
	if(!m_model){
		m_model = new QListDataModel<QObject*>();
	}
	m_model->clear();
	m_model->setParent(this);

	QList<NoticiaHomeWeb*>* lista = noticiasHomeFacade->getLista();
	setStatusBar("Cargando contenido impreso...");//Splash
	if(!lista || lista->size() == 0) {
		qDebug() << "\n\n\n->>>>>>>>>> " << "la lista esta vacia o nula" << "\n\n\n";

		bb::cascades::ImageButton * reloadButton = root->findChild<bb::cascades::ImageButton *>("reloadButton");
		if(!reloadButton) {
			qDebug() << "\n\n\n->>>>>>>>>> " << "No tiene nada el imageButton" << "\n\n\n";
		}else {
			qDebug() << "\n\n\n->>>>>>>>>> " << "Tengo el imageButton" << "\n\n\n";
		}

		setearEscena();
	}else if(lista->size() >  0) {
		NoticiaHomeWeb * object;

		for(int i = 0; i < lista->count() ; i++) {
			object = lista->operator [](i);
			if(i == 0){
				object->setProperty("asignable", "destacada");
			}else{
				object->setProperty("asignable", "listaNormal");
			}
			object -> setIsOnline(true);
			object -> setIsImpreso(false);
			m_model -> append( object );
		}

		if(!m_model_noticias){
			m_model_noticias = new QListDataModel<QObject*>();
		}
		m_model_noticias->clear();
		m_model_noticias->setParent(this);

		for(int i = 0; i < lista->count() ; i++){
			m_model_noticias->append( lista->operator [](i) );
		}

		for(int i = 0; i < m_model->size() ; i++){
			qobject_cast<NoticiaHomeWeb*>(m_model->value(i))->load(false);
		}
	}
	askForHomeNewsPress();
}

void MagazineBB10::cargarNoticiasHomePress() {
	qDebug() << "\n\n\n->>>>>>>>>> " << "Aqui se cargan las noticias del home impreso: " << noticiasHomePressFacade->getLista()->count() << "\n\n\n";

	if(!m_model_noticias_p){
		m_model_noticias_p = new QListDataModel<QObject*>();
	}
	m_model_noticias_p->clear();
	m_model_noticias_p->setParent(this);

	QList<NoticiaHomeWeb*>* lista = noticiasHomePressFacade->getLista();

	setStatusBar(Encode::encodeHex("Cargando aplicación..."));//Splash

	if(!lista || lista->size() == 0) {
		qDebug() << "\n\n\n->>>>>>>>>> " << "la lista esta vacia o nula" << "\n\n\n";

		bb::cascades::ImageButton * reloadButton = root->findChild<bb::cascades::ImageButton *>("reloadButton");
		if(!reloadButton) {
			qDebug() << "\n\n\n->>>>>>>>>> " << "No tiene nada el imageButton" << "\n\n\n";
		}else {
			qDebug() << "\n\n\n->>>>>>>>>> " << "Tengo el imageButton" << "\n\n\n";
		}

		setearEscena();
	}else if(lista->size() >  0) {
		NoticiaHomeWeb * object;

		for(int i = 0; i < lista->count() ; i++){
			object = lista->operator [](i);
			if(i == 0){
				object->setProperty("asignable", "destacada");
			}else{
				object->setProperty("asignable", "listaNormal");
			}
			object -> setIsOnline(false);
			object -> setIsImpreso(true);
			m_model_noticias_p->append( object );
		}

		if(!m_model_noticias_p_2){
			m_model_noticias_p_2 = new QListDataModel<QObject*>();
		}
		m_model_noticias_p_2->clear();
		m_model_noticias_p_2->setParent(this);

		for(int i = 0; i < lista->count() ; i++){
			m_model_noticias_p_2->append( lista->operator [](i) );
		}

		iniciarTimer();

		for(int i = 0; i < m_model_noticias_p->size() ; i++){
			qobject_cast<NoticiaHomeWeb*>(m_model_noticias_p->value(i))->load(false);
		}
	}

}

void MagazineBB10::setearEscena() {
	loginLoader -> setPrincipalCargada(true);
	application->setScene(root);
	emit setSceneComplete();
}

void MagazineBB10::cambioHome() {
	noticiasHome = root->findChild<ListView*>("listaNoticias");
	noticiasHomePress = rootImpreso->findChild<ListView*>("listaNoticias");

	image = root->findChild<ImageView *>("imgDestacada");

	if(!noticiasHomePress){
		qDebug() << "\n\n\n->>>>>>>>>> " << "No tiene nada el noticiasHome" << "\n\n\n";
	}else{
		noticiasHomePress->setDataModel(m_model_noticias);
	}

	if(!noticiasHome){
		qDebug() << "\n\n\n->>>>>>>>>> " << "No tiene nada el noticiasHome" << "\n\n\n";
	}else{
		noticiasHome->setDataModel(m_model);
		setearEscena();
	}
}

void MagazineBB10::cargarSecciones()
{
	askForHomeNews();
	setStatusBar("Descargando noticias..."); //Splash
	if(seccionesFacade && seccionesFacade->getListaSecciones()) {
		addTabs(root);
	} else {
		qDebug() << "\n\n\n->>>>>>>>>> " << "No se agregaron las secciones" << "\n";
		return;
	}
}

void MagazineBB10::askForHomeNews()
{
	noticiasHomeFacade = new NoticiasHomeFacade();
	connect(noticiasHomeFacade, SIGNAL(complete()),this, SLOT(cargarNoticiasHome()));
	connect(noticiasHomeFacade, SIGNAL(error(bool)),this, SLOT(errorNoticiasHome(bool)));
	noticiasHomeFacade -> setBloque ("1");
	noticiasHomeFacade->getInfo();
}

void MagazineBB10::askForHomeNewsPress()
{
	noticiasHomePressFacade = new NoticiasHomeFacade();
	connect(noticiasHomePressFacade, SIGNAL(complete()),this, SLOT(cargarNoticiasHomePress()));
	connect(noticiasHomePressFacade, SIGNAL(error(bool)),this, SLOT(errorNoticiasHomePress(bool)));
	noticiasHomePressFacade -> setBloque ("3");
	noticiasHomePressFacade->getInfo();
}

void MagazineBB10::errorNoticiasHome(bool handle)
{
	if(!handle)
		return;
	fprintf(stderr, "\n\n[MagazineBB10] errorNoticiasHome\n");
	setStatusBar("Cargando contenido impreso...");//Splash
	askForHomeNewsPress();
}

void MagazineBB10::errorNoticiasHomePress(bool handle)
{
	if(!handle)
		return;
	fprintf(stderr, "\n\n[MagazineBB10] errorNoticiasHomePress\n");
	setStatusBar(Encode::encodeHex("Cargando aplicación..."));//Splash
	setearEscena();
	verificarContenido();
}

void MagazineBB10::setVisibilityOnHome(bool visible){
	Container* cont = root->findChild<Container*>("mensajeErrorHome");
	if(cont){
		fprintf(stderr, "\n\n[MagazineBB10] verificarContenido\n");
		cont -> setVisible(visible);
	}else{
		fprintf(stderr, "\n\n[MagazineBB10] verificarContenido no esta\n");
	}
	cont = root->findChild<Container*>("noticiasDelHome");
	if(cont){
		fprintf(stderr, "\n\n[MagazineBB10] verificarContenido2\n");
		cont -> setVisible(!visible);
	}else{
		fprintf(stderr, "\n\n[MagazineBB10] verificarContenido2 no t�\n");
	}
}

void MagazineBB10::verificarContenido(){
	if(!m_model || m_model-> isEmpty() ){
		setVisibilityOnHome(true);
	}else{
		fprintf(stderr, "\n\n[MagazineBB10] verificarContenido - el m_model tiene algo\n");
	}
}

void MagazineBB10::cargarNoticiasHomeRefresh()
{
	if(!m_model){
		m_model = new QListDataModel<QObject*>();
	}
	m_model->clear();
	m_model->setParent(this);

	QList<NoticiaHomeWeb*>* lista = noticiasHomeFacade->getLista();
	if(!lista || lista->size() == 0) {
		qDebug() << "\n\n\n->>>>>>>>>> " << "la lista esta vacia o nula" << "\n\n\n";
		fprintf(stderr, "\n\n[MagazineBB10] cargarNoticiasHomeRefresh\n\n");
	}else if(lista->size() >  0) {
		fprintf(stderr, "\n\n[MagazineBB10] cargarNoticiasHomeRefresh - bien\n\n");
		NoticiaHomeWeb * object;
		for(int i = 0; i < lista->count() ; i++) {
			object = lista->operator [](i);
			if(i == 0){
				object->setProperty("asignable", "destacada");
			}else{
				object->setProperty("asignable", "listaNormal");
			}
			object -> setIsOnline(true);
			object -> setIsImpreso(false);
			m_model -> append( object );
		}
		if(!m_model_noticias){
			m_model_noticias = new QListDataModel<QObject*>();
		}
		m_model_noticias->clear();
		m_model_noticias->setParent(this);
		for(int i = 0; i < lista->count() ; i++){
			m_model_noticias->append( lista->operator [](i) );
		}
		for(int i = 0; i < m_model->size() ; i++){
			qobject_cast<NoticiaHomeWeb*>(m_model->value(i))->load(false);
		}

		setVisibilityOnHome(false);

	}

	errorRefresh();
}

void MagazineBB10::cargarNoticiasHomePressRefresh(){
	if(!m_model_noticias_p){
			m_model_noticias_p = new QListDataModel<QObject*>();
		}
		m_model_noticias_p->clear();
		m_model_noticias_p->setParent(this);

		QList<NoticiaHomeWeb*>* lista = noticiasHomePressFacade->getLista();

		if(!lista || lista->size() == 0) {
			qDebug() << "\n\n\n->>>>>>>>>> " << "la lista esta vacia o nula" << "\n\n\n";
		}else if(lista->size() >  0) {
			NoticiaHomeWeb * object;

			for(int i = 0; i < lista->count() ; i++){
				object = lista->operator [](i);
				if(i == 0){
					object->setProperty("asignable", "destacada");
				}else{
					object->setProperty("asignable", "listaNormal");
				}
				object -> setIsOnline(false);
				object -> setIsImpreso(true);
				m_model_noticias_p->append( object );
			}

			if(!m_model_noticias_p_2){
				m_model_noticias_p_2 = new QListDataModel<QObject*>();
			}
			m_model_noticias_p_2->clear();
			m_model_noticias_p_2->setParent(this);

			for(int i = 0; i < lista->count() ; i++){
				m_model_noticias_p_2->append( lista->operator [](i) );
			}

			for(int i = 0; i < m_model_noticias_p->size() ; i++){
				qobject_cast<NoticiaHomeWeb*>(m_model_noticias_p->value(i))->load(false);
			}
			setVisibilityOnHome(false);
		}
		errorRefresh();
}

void MagazineBB10::refreshHome(){
	if(!m_model || m_model-> isEmpty() ){

		noticiasHomeFacade = new NoticiasHomeFacade();
		connect(noticiasHomeFacade, SIGNAL(complete()),this, SLOT(cargarNoticiasHomeRefresh()));
		connect(noticiasHomeFacade, SIGNAL(error(bool)),this, SLOT(errorRefresh()));
		noticiasHomeFacade -> setBloque ("1");
		noticiasHomeFacade->getInfo();

	}
	if(!m_model_noticias_p || m_model_noticias_p-> isEmpty() ){

		noticiasHomePressFacade = new NoticiasHomeFacade();
		connect(noticiasHomePressFacade, SIGNAL(complete()),this, SLOT(cargarNoticiasHomePressRefresh()));
		connect(noticiasHomePressFacade, SIGNAL(error(bool)),this, SLOT(errorRefresh()));
		noticiasHomePressFacade -> setBloque ("3");
		noticiasHomePressFacade->getInfo();
	}
}

void MagazineBB10::errorRefresh(){
	ActivityIndicator * spin = root->findChild<ActivityIndicator*>("spinnerIndicatorRefresh");
	if(spin) {
		spin->stop();
		spin->setVisible(false);
	}
}

void MagazineBB10::addTabs(TabbedPane * tabContainer)
{
	QString texto = "";

	if(!seccionesFacade->getListaSecciones()) {
		qDebug() << "\n\n->>>>>>>>>>La lista de tabs esta vacia." << "\n\n";
		return;
	}

	int tam = 0;

	if(!seccionesFacade->getListaSecciones()->count()) {
		qDebug() << "\n\nNo se cargaron las secciones." << "\n\n";
		levantarPopup("No se cargaron las secciones.");
		return;
	}else
		tam = seccionesFacade->getListaSecciones()->count();

	if(!tam) {
		qDebug() << "\n\n->Salio por tam de addTabs inexistente" << "\n\n";
		return;
	}else if(tam < 1) {
		qDebug() << "\n\n->Salio por tam de addTabs menor que 1" << "\n\n";
		return;
	}

	int j = 0;
	for(int i = 0; i < tam ; i++) {

		texto = seccionesFacade->getListaSecciones()->operator [](i)->getId();

		if( texto == "1" ||			//Confidenciales
				texto == "3" ||		//Naci�n
				texto == "4" ||		//Econom�a
				texto == "5" ||		//Mundo
				texto == "6" ||		//Cultura
				texto == "7" ||		//Vida moderna
				texto == "8" ||		//Gente
				texto == "14" ||		//Opini�n
				texto == "45"		//Deportes
		)
		{
			QmlDocument *qmlfor = QmlDocument::create("asset:///TemplateSecciones.qml");
			qmlfor->setProperty("idSeccion", seccionesFacade->getListaSecciones()->
					operator [](i)->getId());
			qmlfor->setContextProperty("mainApp", this);
			qmlfor->setContextProperty("share", share);
			qmlfor->setContextProperty("loginLoader", loginLoader);
			qmlfor -> setContextProperty("favorites", favorites);
			qmlfor ->setContextProperty("publicidadEngine", engine);
			NavigationPane* page = qmlfor->createRootObject<NavigationPane>();
			Tab* tab = Tab::create()
			.title(upperToLower(seccionesFacade->getListaSecciones()->operator [](i)->getNombre()))
			.objectName(seccionesFacade->getListaSecciones()->
					operator [](i)->getId());

			// Tab confidenciales
			if(texto == "1" ){
				tab->setImageSource(QUrl("asset:///images/icon_confidenciales.png"));
				tab->setContent(page);
				tabContainer->insert(3, tab);
				if(tabsPress){
					tabsPress -> insert(3, tab);
				}
				if(tabsOnline){
					tabsOnline -> insert(3, tab);
				}
			}else{
				tab->setImageSource(QUrl("asset:///images/icon_quienes.png"));
				tab->setContent(page);

				if(texto == "14"){
					tab->setTitle("Columnistas");
					tabContainer-> insert(4,tab);
				}else{
					tabContainer-> insert(4 + j,tab);
				}
				if(tabsPress) {
					//La impresa no tiene seccion de Deportes
					if(texto != "45")
						tabsPress -> insert(4 + j,tab);
				}
				if(tabsOnline) {
					tabsOnline -> insert(4 + j,tab);
				}
				j++;
			}

			if(texto == "1") {
				insertarEnHash("1", true, true, "Confidenciales", true);
			}else if(texto == "3"){
				insertarEnHash("3", true, true, "Nación", true);
			}else if(texto == "4"){
				insertarEnHash("4", true, true, "Economía", true);
			}else if(texto == "5"){
				insertarEnHash("5", true, true, "Mundo", true);
			}else if(texto == "6"){
				insertarEnHash("6", true, true, "Cultura", true);
			}else if(texto == "7"){
				insertarEnHash("7", true, true, "Vida moderna", true);
			}else if(texto == "8"){
				insertarEnHash("8", true, true, "Gente", true);
			}else if(texto == "45"){
				insertarEnHash("45", true, false, "Deportes", true);
			}

			else if(texto == "14"){
				insertarEnHash("14", true, false, "Columnistas", true);
			}

		}else{
			// No se agregan algunas de las categor�as
		}
	}

}

void MagazineBB10::insertarEnHash(QString id, bool online, bool impreso, QString nombre, bool globalMode){
	Seccion * seccion = new Seccion();
	seccion -> setId(id);
	seccion -> setOnline(online);
	seccion -> setImpreso(impreso);
	seccion -> setNombre(nombre);
	seccion -> setOnlineMode(globalMode);

	hashSecciones.insert(id, seccion );
}

void MagazineBB10::addStaticPressTabs()
{
	QmlDocument *qmlfor = QmlDocument::create("asset:///TemplateCaricaturas.qml");
	qmlfor->setContextProperty("mainApp", this);
	qmlfor->setContextProperty("share", share);
	qmlfor->setContextProperty("loginLoader", loginLoader);
	NavigationPane* page = qmlfor->createRootObject<NavigationPane>();
	Tab* tab = Tab::create()
	.title("Caricaturas")
	.objectName("caricaturas");
	tab->setImageSource(QUrl("asset:///images/icon_calendar.png"));
	tab->setContent(page);
	root -> add(tab);

	insertarEnHash("34", false, true, "Caricaturas", false);

	qmlfor = QmlDocument::create("asset:///TemplateSecciones.qml");
	qmlfor->setContextProperty("mainApp", this);
	qmlfor->setContextProperty("share", share);
	qmlfor->setContextProperty("loginLoader", loginLoader);
	qmlfor ->setContextProperty("publicidadEngine", engine);
	page = qmlfor->createRootObject<NavigationPane>();
	tab = Tab::create()
	.title("Enfoque")
	.objectName("enfoque");
	tab->setImageSource(QUrl("asset:///images/icon_calendar.png"));
	tab->setContent(page);
	root -> add(tab);

	insertarEnHash("2", false, true, "Enfoque", false);

	qmlfor = QmlDocument::create("asset:///TemplateSecciones.qml");
	qmlfor->setContextProperty("mainApp", this);
	qmlfor->setContextProperty("share", share);
	qmlfor->setContextProperty("loginLoader", loginLoader);
	qmlfor ->setContextProperty("publicidadEngine", engine);
	page = qmlfor->createRootObject<NavigationPane>();
	tab = Tab::create()
	.title("Opinión")
	.objectName("opinion");
	tab->setImageSource(QUrl("asset:///images/icon_calendar.png"));
	tab->setContent(page);
	root -> add(tab);

	insertarEnHash("14", false, true, "Opinión", false);

	qmlfor = QmlDocument::create("asset:///TemplateSociedad.qml");
	qmlfor->setContextProperty("mainApp", this);
	qmlfor->setContextProperty("share", share);
	qmlfor->setContextProperty("loginLoader", loginLoader);
	qmlfor ->setContextProperty("publicidadEngine", engine);
	page = qmlfor->createRootObject<NavigationPane>();
	tab = Tab::create()
	.title("Sociedad")
	.objectName("sociedad");
	tab->setImageSource(QUrl("asset:///images/icon_calendar.png"));
	tab->setContent(page);
	root -> add(tab);

	insertarEnHash("12", false, true, "Sociedad", false);

}

/**
 *  Env�a la petici�n del contenido del Tab Din�mico que est� activo actualmente
 */
void MagazineBB10::CargarDatosSecciones() {

	Tab* tab = root->activeTab();
	if(!tab){
		levantarPopup( "Ha ocurrido un error." );
		return;
	}

	if(tab -> objectName() == "quienesSomos"
			|| tab -> objectName() == "contacto"
					|| tab -> objectName() == "acercaDe")
	{
		return;
	}else if (tab -> objectName() == "TabOnlinePress") {
		impresa -> loadPage(root);
		fillUpPressList();
		return;
	}

	if(!tab->title().isNull() && !tab->title().isEmpty()) {

		if(tab->title() == "Buscar"
				|| tab->title() == "Favoritos"
						|| tab->title() == "Inicio"
								|| tab->title() == "Alertas") {
			return;
		}else if (tab -> title() == "Indicadores") {
			indicadores -> cargarIndicadores();
			return;
		}else if (tab -> title() == "Fotos") {
			galeriaFotos -> cargarGalerias();
			return;
		}else if (tab -> title() == "Videos") {
			galeriaVideos -> cargarGalerias();
			return;
		}

	}

	// Si llega hasta aqu�, significa que es una secci�n din�mica o hay alguna cableada
	QList<Seccion*>* list = seccionesFacade->getListaSecciones();

	Seccion* estaSecc;

	int tam = list->count();

	for(int i = 0 ; i < tam ; i++) {
		if(tab->title() == upperToLower( list->operator [](i)->getNombre() ))
		{
			estaSecc = list->operator [](i);

			qDebug() << "\n\n\nCargarDatosSecciones tiene::: " << estaSecc -> getNombre() << "\n\n\n" ;
			break;
		}else if(tab -> title() == "Columnistas"){
			if(upperToLower( list->operator [](i)->getNombre()) == "Opinión"){
				estaSecc = list->operator [](i);
				qDebug() << "\n\nNombre: " << estaSecc -> getNombre() << "\n\n";
				break;
			}
		}
	}

	if(!estaSecc){
		qDebug() << "\n\nAlgo salio mal con la seleccion de secciones\n\n";
		return;
	}

	sectionsInfoFacade = new SectionsInfoFacade();
	connect(sectionsInfoFacade, SIGNAL(complete()),this, SLOT(cargarSeccionInfo()));

	if(tab->title().isNull() || tab->title().isEmpty()) {
		qDebug() << "\n\n- CargarDatosSecciones vacio " << "\n\n";
	} else {
		qDebug() << "\n\n- Pidiendo los datos en la seccion: " << tab->title() << "\n\n";
		QList<Seccion*>* list = seccionesFacade->getListaSecciones();
		if (list) {
			qDebug() << "\n\n->Entro en lista de secciones" << "\n\n";
			int tam = list->count();
			analyticsEventsNot(tab->title());

			Seccion* seccion;

			for(int i = 0 ; i < tam ; i++) {

				if(tab -> title() == "Columnistas"){
					seccion = list->operator [](i);
					Seccion * seccionHash = hashSecciones.value(seccion -> getId());
					if(estaSecc -> isOnlineMode()) {
						if(seccionHash -> getModelO() && ! seccionHash -> getModelO() -> isEmpty()) {
							qDebug() << "\n\n->1. Ya pedi el contenido, no lo hago de nuevo. " << "\n\n";
							
						}
					}
					else {
						if(seccionHash -> getModelI() && ! seccionHash -> getModelI() -> isEmpty()) {
							qDebug() << "\n\n->2. Ya pedi el contenido, no lo hago de nuevo. " << "\n\n";
							
						}
					}
					spin = tab->findChild<ActivityIndicator*>("spinner");
					if(spin) {
						spin->start();
						spin->setVisible(true);
					}
					sectionsInfoFacade->getInfo("14", true);
					return;
				}
				seccion = list->operator [](i);
				Seccion * seccionHash = hashSecciones.value(seccion -> getId());
				if(tab->title() == upperToLower(seccion->getNombre())) {
					if(estaSecc -> isOnlineMode()) {
						if(seccionHash -> getModelO() && ! seccionHash -> getModelO() -> isEmpty()) {
							qDebug() << "\n\n->1. Ya pedi el contenido, no lo hago de nuevo. " << "\n\n";
							
						}
					}
					else {
						if(seccionHash -> getModelI() && ! seccionHash -> getModelI() -> isEmpty()) {
							qDebug() << "\n\n->2. Ya pedi el contenido, no lo hago de nuevo. " << "\n\n";
							
						}
					}
					spin = tab->findChild<ActivityIndicator*>("spinner");
					if(spin) {
						spin->start();
						spin->setVisible(true);
					}
					sectionsInfoFacade->getInfo(seccion->getId(), estaSecc -> isOnlineMode());
					break;
				}
			}
			//hashSecciones[estaSecc -> getId()] = estaSecc;
		} else {
			// No se lograron cargar las secciones
			qDebug() << "\n\n->No se lograron cargar las secciones " << "\n\n";
			levantarPopup("Las secciones no fueron cargadas. Intente nuevamente.");
		}
	}
}


/**
 *  Toma la info obtenida y la pinta en un tab
 */
void MagazineBB10::cargarSeccionInfo() {

	Tab* tab = root->activeTab();

	if( !spin ) {
		qDebug() << "\n\n->>>>>>>>>>No spinner " << "\n\n";
	} else {
		qDebug() << "\n\n->>>>>>>>>>Spinner off " << "\n\n";

		QList<Seccion*>* list = seccionesFacade->getListaSecciones();

		Seccion* estaSecc;

		int tam = list->count();

		int i = 0;
		for(i = 0 ; i < tam ; i++) {
			qDebug() << "\n\n" <<  list->operator [](i) -> getNombre() << "\n\n";

			if(tab->title() == "Columnistas"){
				estaSecc = list->operator [](i);
				qDebug() << "\n\n\nEl condenado id de columnistas es dice:: "<< estaSecc -> getId() <<"\n\n\n" ;
				//fprintf(stdout, "" + estaSecc -> getId() );
				break;
			}

			if(tab->title() == upperToLower( list->operator [](i)->getNombre() ))
			{
				estaSecc = list->operator [](i);
				break;
			}
		}

		if(estaSecc) {
			//Aqui colocamos la informaci�n en el datamodel de las secciones
			QListDataModel<QObject*>* m_model_secciones;

			estaSecc -> setOnline( hashSecciones .value(estaSecc -> getId()) -> isOnline()  );
			estaSecc -> setImpreso( hashSecciones .value(estaSecc -> getId()) -> isImpreso()  );

			m_model_secciones = new QListDataModel<QObject*>();
			m_model_secciones->setParent(this);

			QList<NoticiaHomeWeb*>* lista = sectionsInfoFacade->getLista();
			NoticiaHomeWeb * object;

			int tam = lista->size();

			if(!tam) {
				//estaSecc->setObtained(false);
				levantarPopup("Ha ocurrido un error. Intente nuevamente.");
			}else {
				if(estaSecc -> getId() != "34" && estaSecc -> getId() != "12") {
					for(int i = 0; i < tam ; i++) {
						object = lista->operator [](i);
						if(i == 0
								&& tab->title() != "Caricaturas"
										&& tab->title() != "Sociedad"	)
						{
							object->setProperty("asignable", "destacada");
						}else {
							object->setProperty("asignable", "listaNormal");
						}
						object -> setIdSeccion(estaSecc -> getId());
						object -> setIsOnline( estaSecc -> isOnline() );
						object -> setIsImpreso( estaSecc -> isImpreso() );

						object -> s_setIsOnline(estaSecc -> isOnlineMode());
						object -> s_setIsImpreso(!estaSecc -> isOnlineMode());

						if(tab->title() == "Columnistas"){
							object -> setSeccion("Columnistas");
						}else{
							qDebug() << "\n\n\nEl idSeccion dice:: "<< estaSecc -> getId() <<"\n\n\n" ;
						}

						m_model_secciones->append( object );
					}

					tab -> findChild<ListView* >("listaNoticiasSecciones")
													-> setDataModel(m_model_secciones);
				}else {
					for(int i = 0; i < tam ; i++) {
						object = lista->operator [](i);
						object->setProperty("asignable", "listaNormal");
						object -> setIdSeccion(estaSecc -> getId());
						object -> setIsOnline( estaSecc -> isOnline() );
						object -> setIsImpreso( estaSecc -> isImpreso() );

						object -> s_setIsOnline(estaSecc -> isOnlineMode());
						object -> s_setIsImpreso(!estaSecc -> isOnlineMode());

						m_model_secciones->append( object );
					}
					tab -> findChild<ListView* >("listaGaleriaCaricaturas")
													-> setDataModel(m_model_secciones);
				}

				if(estaSecc -> isOnlineMode()) {
					estaSecc -> setModelO(m_model_secciones);
					levantarPopup("Modo Online");
				}else {
					estaSecc -> setModelI(m_model_secciones);
					levantarPopup("Modo Impreso");
				}

				for(int i = 0; i < m_model_secciones->size() ; i++) {
					// True si es de estas secciones y false si no lo es
					qobject_cast<NoticiaHomeWeb*>
					(m_model_secciones->value(i))->load(
							tab->title() != "Caricaturas"
									&& tab->title() != "Sociedad");
				}
			}

			hashSecciones[estaSecc -> getId()] = estaSecc;

		}else {
			// no hay nada
		}

		spin->stop();
		spin->setVisible(false);
	}
}

QString MagazineBB10::modoSeccion(QString id){
	QString modo = "-1";
	qDebug() << "\n\n\nEl id dice:: "<< id <<"\n\n\n" ;
	if(id == "1" ||
			id == "3" ||
			id == "4" ||
			id == "5" ||
			id == "6" ||
			id == "7" ||
			id == "8"
	){
		// Ambos
		modo = "1";
	}else if (id == "45") {
		// Online
		modo = "2";
	}else if (id == "34" ||
			id == "2" ||
			id == "14" ||
			id == "12") {
		// Impreso
		modo = "3";
	}else {
		qDebug() << "\n\n\nNo hice una shit\n\n\n" ;
	}
	return modo;
}

/**
 */
void MagazineBB10::cambiarImpresoOnlineSecciones(QString idSeccion, bool onlineImpreso) {

	Seccion * seccion = hashSecciones.value(idSeccion);
	QString mode = modoSeccion(idSeccion);
	Tab* tab = root->activeTab();
	NavigationPane * nav = tab->findChild<NavigationPane*>("navPaneTemplate");

	if(nav) {
		nav -> pop();
	}

	if(!seccion) {
		qDebug() << "\n\n\nNo es una seccion, esto se rompio..." << "\n\n\n";
		return;
	}

	if (mode != "1") {
		levantarPopup(Encode::encodeHex("Esta sección tiene una única modalidad"));
		return;
	}else if(mode == "-1") {
		qDebug() << "\n\n\nNo hay modalidad, esto se rompio" << "\n\n\n";
		return;
	}
	modo_o_i_secciones = onlineImpreso;
	if(onlineImpreso) {
		if( seccion -> getModelO() && ! seccion -> getModelO() -> isEmpty() ) {
			tab -> findChild<ListView* >("listaNoticiasSecciones")
											-> setDataModel(seccion -> getModelO() );
		}else {
			levantarPopup("Espere un momento");
			seccion -> setOnlineMode(true);
			CargarDatosSecciones();
		}
	}else {
		if( seccion -> getModelI() && ! seccion -> getModelI() -> isEmpty() ) {
			tab -> findChild<ListView* >("listaNoticiasSecciones")
											-> setDataModel(seccion -> getModelI() );
		}else {
			levantarPopup("Espere un momento");
			seccion -> setOnlineMode(false);
			CargarDatosSecciones();
		}
	}
	hashSecciones[seccion -> getId()] = seccion;

}

void MagazineBB10::loadNewsList() {
	if(newsList) {
		newsList->setDataModel(m_model_noticias);
	}
}

void MagazineBB10::sendSearchAction(QString textoBuscar, int pagina, int tipo){
	// Facade de b�squeda
	busquedaFacade = new BusquedaFacade();
	connect(busquedaFacade, SIGNAL(complete()),this, SLOT(cargarBusqueda()));
	busquedaFacade->getInfo(textoBuscar, pagina, tipo);
}

void MagazineBB10::cargarBusqueda() {

	m_model_busqueda = new QListDataModel<QObject*>();
	m_model_busqueda->setParent(this);

	QList<NoticiaHomeWeb*>* lista = busquedaFacade->getLista();
	NoticiaHomeWeb * object;

	for(int i = 0; i < lista->count() ; i++) {
		object = lista->operator [](i);
		object->setProperty("asignable", "listaNormal");

		m_model_busqueda->append( object );
	}
	root->findChild<ListView*>("listaBusqueda")->setDataModel(m_model_busqueda);

	for(int i = 0; i < m_model_busqueda->size() ; i++) {
		qobject_cast<NoticiaHomeWeb*>(m_model_busqueda->value(i))->load(false);
	}

	spinBuscar = root->findChild<ActivityIndicator*>("spinnerBuscar");
	spinBuscar->stop();

}

void MagazineBB10::mostrarIndividual( NoticiaHomeWeb *chosenItem ) {
	//Tomar el chosenItem para solicitar la informaci�n faltante y d�rsela
	//al datamodel que tiene la ventana actual

	//Solicitar la informaci�n:
	completarArticuloFacade = new CompletarArticuloFacade(chosenItem);
	connect(completarArticuloFacade, SIGNAL(complete()),this, SLOT(CompletarArticulo()));
	completarArticuloFacade->getInfo(chosenItem->getId());

	elArticulo = chosenItem;

}

void MagazineBB10::CompletarArticulo() {
	elArticulo->setContenidoLimpio(completarArticuloFacade->getElContenido());

	for(int i = 0; i < m_model_busqueda->size() ; i++){
		if(elArticulo->getId().compare(qobject_cast<NoticiaHomeWeb*>(m_model_busqueda->value(i))->getId()) == 0){
			qobject_cast<NoticiaHomeWeb*>(m_model_busqueda->value(i))->setContenidoLimpio(elArticulo->getContenidoLimpio());
		}
	}
	
}

void MagazineBB10::levantarPopup(QString texto) {
	bb::system::SystemToast *dialog = new bb::system::SystemToast();
	dialog->setBody(texto);
	dialog->show();
}

void MagazineBB10::salvarNoticia(NoticiaHomeWeb *noticiaSeleccionada) {
	this->noticiaSeleccionada = noticiaSeleccionada;
	DataStoreConfig *store = new DataStoreConfig();
	store->save(noticiaSeleccionada);
	levantarPopup("Se ha almacenado satisfactoriamente la noticia.");
}

void MagazineBB10::eliminarNoticia(NoticiaHomeWeb *noticiaSeleccionada) {
	this->noticiaSeleccionada = noticiaSeleccionada;
	DataStoreConfig *store = new DataStoreConfig();
	store->removeNews(noticiaSeleccionada->getId());
	levantarPopup("Se ha eliminado satisfactoriamente la noticia.");
	loadFavs();
}

void MagazineBB10::eliminarTodas() {
	DataStoreConfig *store = new DataStoreConfig();
	store->remove();
	limpiarDataModelFavoritos();

	levantarPopup("Se ha eliminado satisfactoriamente todo el contenido.");
}

void MagazineBB10::limpiarDataModelFavoritos() {
	ListView * listViewFavs = root->findChild<ListView *>("listaFavoritos");
	if(listViewFavs){
		listViewFavs->resetDataModel();
	}
	mostrarEtiqueta("No has agregado noticias a tus Favoritos", true);
}

void MagazineBB10::loadFavs() {
	DataStoreConfig *store = new DataStoreConfig();
	QString neededWord = "";

	if(store){
		QListDataModel<QObject*>* m_model_favoritos;
		m_model_favoritos = new QListDataModel<QObject*>();
		m_model_favoritos->setParent(this);
		ListView * listViewFavs = root->findChild<ListView *>("listaFavoritos");

		if(store->hasKeys()) {
			for(int i = 0; i < store->keysLength() ; i += store->AttributesNumber()){
				QStringList pieces = (store->getKeys()).at(i).split( "/" );
				neededWord = pieces.value( 0 );
				NoticiaHomeWeb * object;
				object = store->getNoticiaSalvada(neededWord);
				object->setProperty("asignable", "listaNormal");
				m_model_favoritos->append( object );
			}

			for(int i = 0; i < m_model_favoritos->size() ; i++) {
				qobject_cast<NoticiaHomeWeb*>(m_model_favoritos->value(i))->load(true);
			}

			if(listViewFavs){
				listViewFavs->setDataModel(m_model_favoritos);
			}
			mostrarEtiqueta("", false);
		}else{
			//Se debe indicar a la pantalla que est� vac�a la lista de noticias favoritas
			if(listViewFavs){
				listViewFavs->resetDataModel();
				listViewFavs->setDataModel(m_model_favoritos);
			}
			mostrarEtiqueta("No has agregado noticias a tus Favoritos", true);
		}
	}else{
		levantarPopup("Ha ocurrido un error. Intente nuevamente.");
	}
}

void MagazineBB10::mostrarEtiqueta(QString texto, bool mostrar) {
	Label * etiquetaFavs = root->findChild<Label *>("etiquetaFavs");
	etiquetaFavs->setText(texto);
	etiquetaFavs->setVisible(mostrar);
}

QByteArray MagazineBB10::encodeQString(const QString& toEncode){
	return toEncode.toUtf8();
}

void MagazineBB10::goHome() {
	root -> setActiveTab( root -> at(0) );
	NavigationPane * navPane = root->findChild<NavigationPane *>("navPaneNoticiero");
	if(navPane){
		if(navPane->count() > 1)
			navPane->pop();
	}
	qDebug() << "\n\n\nHash size:: " << hashSecciones.size() << "\n\n\n";
}

void MagazineBB10::goAlertas() {

	Sheet *sheet = root->findChild<Sheet *>("alertasSheet");
	if (sheet) {
		sheet->open();
	}
	qDebug("\n\n[ApplicationUI] goAlertas\n");
	fprintf(stderr, "\n\n[ApplicationUI] goAlertas\n");


}

void MagazineBB10::loginFinished() {

}

QString MagazineBB10::upperToLower(QString cadena)
{
	QString aux = cadena;
	QString head = aux.remove(1, aux.size()-1);
	QString tail = (cadena.remove(0,1)).toLower();
	return head + tail;
}

void MagazineBB10::pushGalleryOnDetail(int valor)
{
	QListDataModel<QObject*>* modelo = galeriaFotos->getModel();
	qDebug() << "\n\n\nV:: " << qobject_cast<Multimedia*>(modelo->value(valor))->getNombre() << "\n\n\n";
	NavigationPane * nav = root->findChild<NavigationPane *>("navPaneGalerias");
	analyticsEventsDetalle("Fotos",qobject_cast<Multimedia*>(modelo->value(valor))->getNombre());
	if(nav) {

		nav->push(pagina);
		ListView * lista = root->findChild<ListView *>("listaGaleriasDetalle");
		if(lista) {
			// Aqu� dentro se construye el datamodel de Fotos
			QListDataModel<QObject*>* modelo_fotos = new QListDataModel<QObject*>();
			Foto* object;
			QList<Foto *>* listaFotos = qobject_cast<Multimedia*>(modelo->value(valor))->getMultimedia();
			for(int i = 0; i < listaFotos->count() ; i++) {
				object = listaFotos->operator [](i);
				object -> load(true);
				modelo_fotos->append( object );
			}
			lista->resetDataModel();
			lista->setDataModel(modelo_fotos);

			ActivityIndicator * spinner = root->findChild<ActivityIndicator *>("spinnerDetalle");
			if(spinner) {
				spinner->setVisible(true);
			}
		}

		Label * title = root->findChild<Label *>("tituloGaleria");
		if(title){
			title->setText(qobject_cast<Multimedia*>(modelo->value(valor))->getNombre());
		}

	}
}

void MagazineBB10::pushVideoDetailPage(QString idVideo, QString idMultimedia, QString imagenPequenia) {

	imagenPequeniaVideo = QString("http://www.Magazine.com/") + imagenPequenia;

	qDebug() << "\n\nidVideo:: " << idVideo << " idMultimedia:: " << idMultimedia << "\n\n";
	askForIndividualVideo(idVideo, idMultimedia);
}

void MagazineBB10::askForIndividualVideo(QString idVideo, QString idMultimedia) {
	individualVideoFacade = new VideoIndividualFacade();
	connect(individualVideoFacade, SIGNAL(complete()),this, SLOT(cargarVideoIndividual()));
	individualVideoFacade->getInfo(idVideo, idMultimedia);

	ActivityIndicator * spinner = root->findChild<ActivityIndicator *>("spinnerVideoDetalle");

	if (spinner) {
		spinner->start();
		spinner->setVisible(true);
	}

}

void MagazineBB10::cargarVideoIndividual() {
	//Cargar toda la informaci�n en la ventana de video individual

	video = individualVideoFacade -> IndividualVideo();

	const char* texto = ( QString("http://www.Magazine.com") + video -> getUrlMedia()) . toStdString().c_str() ;
	QString titulo = video -> getTitulo();
	analyticsEventsDetalle("Videos",titulo);
	ActivityIndicator * spinner = root->findChild<ActivityIndicator *>("spinnerVideoDetalle");

	if (spinner) {
		spinner->stop();
		spinner->setVisible(false);
	}

	navigator_invoke(texto, 0);

}

void MagazineBB10::setImageViewVideo() {
	qDebug() << "\n\n\n\nEstamos en el setImageViewVideo\n\n\n\n";
	ImageView * imagenVideo = root->findChild<ImageView *>("imagenVideo");
	if (imagenVideo) {
		qDebug() << "\n\n\n\nPuse la imagen\n\n\n\n";
		imagenVideo -> setImage(video -> imagen());
	}else {
		qDebug() << "\n\n\n\nNo puse la imagen\n\n\n\n";
	}
}

/**
 *  Define el modo en el que se est� mostrando el app:
 *
 *  1. online
 *  2. impreso
 *  3. ambos
 *
 */
QString MagazineBB10::whichMode() {
	if(modo == 1){
		return "Edición Online";
	}else if(modo == 2){
		return "Edición Impresa";
	}
	return "";
}

QString MagazineBB10::nextMode() {
	if(modo == 1){
		return "Edición Impresa";
	}else if(modo == 2){
		return "Edición Online";
	}
	return "";
}

void MagazineBB10::changeOnlinePress() {
	qDebug() << "\n\n\nDebo cambiar a otro modo" << "\n\n\n" ;
	Tab * tab;
	// Cambiar las secciones
	// Cambiar las noticias del home
	if(modo == 1) {
		modo = 2;
		tab = root->findChild<Tab *>("TabOnlinePress");
	}else if(modo == 2) {
		modo = 1;
		tab = rootImpreso->findChild<Tab *>("TabOnlinePress");
	}

	if(tab){
		tab -> setTitle(nextMode());
	}

}

void MagazineBB10::setTabs() {
	//Aqu� cambiamos entre conjuntos de Tabs

	qDebug() << "\n\n\nEl conteo es:: " << root -> count() << "\n\n\n";

	int tam = root -> count();
	for(int i = 7; i < tam ; i++){
		qDebug() << "\n\n\nLa i vale:: " << i << "\n\n\n";
		root -> remove( root -> at(i) );
	}

	qDebug() << "\n\n\nEl conteo es 2:: " << root -> count() << "\n\n\n";
	return;
	if(modo == 2){
		
		for(int i = 0; i < tabsOnline -> count() ; i++){
			root -> insert(i, tabsOnline -> at(i) );
		}
		modo = 1;
	}else if(modo == 1){
		
		for(int i = 0; i < tabsPress -> count() ; i++){
			root -> insert(i, tabsPress -> at(i) );
		}

		modo = 2;
	}else{
		qDebug() << "\n\n\n" << "Algo ha salido mal en el setTabs" << "\n\n\n";
	}
}

/**
 */
void MagazineBB10::openLoginSheet() {
	
	Sheet *sheet = root->findChild<Sheet *>("configurationLoginSheet");
	if (sheet) {
		sheet->open();
	}
}

void MagazineBB10::registrarAlertas()
{

	pushApi->initializePushSession();
	pushApi->cargarRegistro();
	pushApi->setTabSeleccionado(true);

	if (!loginLoader->isLogin()) {
		Sheet *sheet = root->findChild<Sheet *>("configurationLoginSheet");
		if (sheet) {
			sheet->open();

		}
	}

}

void MagazineBB10::desRegistrarAlertas()
{
	fprintf(stderr, "\n MagazineBB10::desRegistrarAlertas \n");
	loginLoader->desloguear();
}





void MagazineBB10::setHomeMode(bool online) {
	ImageButton * boton = root -> findChild<ImageButton *>("boton_impreso_home");
	if(boton){
		boton -> setEnabled(online);
		qDebug() << "\n\n\nImpreso se encontro" << "\n\n\n";
	} else {
		qDebug() << "\n\n\nImpreso no se encontro" << "\n\n\n";
	}

	boton = root -> findChild<ImageButton *>("boton_online_home");
	if(boton){
		boton -> setEnabled(!online);
		qDebug() << "\n\n\nOnline se encontro" << "\n\n\n";
	}else {
		qDebug() << "\n\n\nOnline no se encontro" << "\n\n\n";
	}
}

void MagazineBB10::fillUpPressList() {
	qDebug() << "\n\nFillUpPressList\n\n" ;

	ListView * lista = root -> findChild<ListView *>("listaPrincipalImpresa");

	Seccion * object;

	if(lista) {

		QListDataModel<QObject*>* m_model_secciones;
		m_model_secciones = new QListDataModel<QObject*>();
		m_model_secciones->setParent(this);

		object = new Seccion();
		object -> setId("208");
		object -> setNombre("100 Empresas");
		object->setProperty("asignable", "");
		m_model_secciones->append( object );

		object = new Seccion();
		object -> setId("34");
		object -> setNombre("Caricaturas");
		object->setProperty("asignable", "");
		m_model_secciones->append( object );

		object = new Seccion();
		object -> setId("1");
		object -> setNombre("Confidenciales");
		object->setProperty("asignable", "");
		m_model_secciones->append( object );

		object = new Seccion();
		object -> setId("6");
		object -> setNombre("Cultura");
		object->setProperty("asignable", "");
		m_model_secciones->append( object );

		object = new Seccion();
		object -> setId("4");
		object -> setNombre(Encode::encodeHex("Economía"));
		object->setProperty("asignable", "");
		m_model_secciones->append( object );

		object = new Seccion();
		object -> setId("2");
		object -> setNombre("Enfoque");
		object->setProperty("asignable", "");
		m_model_secciones->append( object );

		object = new Seccion();
		object -> setId("8");
		object -> setNombre("Gente");
		object->setProperty("asignable", "");
		m_model_secciones->append( object );

		object = new Seccion();
		object -> setId("5");
		object -> setNombre("Mundo");
		object->setProperty("asignable", "");
		m_model_secciones->append( object );

		object = new Seccion();
		object -> setId("3");
		object -> setNombre(Encode::encodeHex("Nación"));
		object->setProperty("asignable", "");
		m_model_secciones->append( object );

		object = new Seccion();
		object -> setId("10");
		object -> setNombre(Encode::encodeHex("Opinión"));
		object->setProperty("asignable", "");
		m_model_secciones->append( object );

		object = new Seccion();
		object -> setId("12");
		object -> setNombre("Sociedad");
		object->setProperty("asignable", "");
		m_model_secciones->append( object );

		object = new Seccion();
		object -> setId("7");
		object -> setNombre("Vida Moderna");
		object->setProperty("asignable", "");
		m_model_secciones->append( object );

		lista -> setDataModel(m_model_secciones);

	}else{
		
	}

}

void MagazineBB10::RaisePressNewsList(QString id, QString nombre) {
	
	sectionsInfoFacade = new SectionsInfoFacade();
	connect(sectionsInfoFacade, SIGNAL(complete()),this, SLOT(cargarPressInfo()));
	sectionsInfoFacade->getInfo(id, false);
	// Debe levantar la ventana y colocarle el nombre de la secci�n

	pressPage = qmlImpreso->createRootObject<Page>();

	idSeccImpresa = id;

	NavigationPane* navpane = root -> findChild<NavigationPane *>("Impresa");
	if(navpane){
		navpane -> push(pressPage);
	}

	ActivityIndicator* spinner = pressPage -> findChild<ActivityIndicator *>("spinnerImpreso");

	if(spinner){
		spinner -> start();
		spinner -> setVisible(true);
	}

}

void MagazineBB10::cargarPressInfo() {
	QList<NoticiaHomeWeb*>* lista = sectionsInfoFacade->getLista();
	NoticiaHomeWeb * object;

	int tam = lista->size();

	NavigationPane* navpane = root -> findChild<NavigationPane *>("Impresa");
	if(!navpane){
		return;
	}

	if(!tam || tam <= 0) {
		levantarPopup("Ha ocurrido un error. Intente nuevamente.");
	}else {

		QListDataModel<QObject*>* m_model_detalle;
		m_model_detalle = new QListDataModel<QObject*>();
		m_model_detalle->setParent(this);

		if(idSeccImpresa != "34" && idSeccImpresa != "12") {
			for(int i = 0; i < tam ; i++) {
				object = lista->operator [](i);
				if(i == 0
						&& idSeccImpresa != "12"
								&& idSeccImpresa != "34"	)
				{
					object->setProperty("asignable", "destacada");
				}else {
					object->setProperty("asignable", "listaNormal");
				}
				object -> setIdSeccion(idSeccImpresa);
				object -> setIsOnline( false );
				object -> setIsImpreso( true );

				object -> s_setIsOnline(false);
				object -> s_setIsImpreso(true);

				m_model_detalle->append( object );
			}

			pressPage -> findChild<ListView* >("listaNoticiasSeccionesImpresas")
																		-> setDataModel(m_model_detalle);
		}else {
			for(int i = 0; i < tam ; i++) {
				object = lista->operator [](i);
				object->setProperty("asignable", "listaNormal");
				object -> setIdSeccion(idSeccImpresa);
				object -> setIsOnline( false );
				object -> setIsImpreso( true );

				object -> s_setIsOnline(false);
				object -> s_setIsImpreso(true);

				m_model_detalle->append( object );
			}
			pressPage -> findChild<ListView* >("listaNoticiasSeccionesImpresas")
																		-> setDataModel(m_model_detalle);
		}

		ActivityIndicator* spinner = pressPage -> findChild<ActivityIndicator *>("spinnerImpreso");

		if(spinner){
			spinner -> stop();
			spinner -> setVisible(false);
		}

		for(int i = 0; i < m_model_detalle->size() ; i++) {
			// True si es de estas secciones y false si no lo es
			qobject_cast<NoticiaHomeWeb*> (m_model_detalle -> value(i)) -> load(false);
		}

		setCurrentDataModel(m_model_detalle);

	}
}

void MagazineBB10::setCurrentDataModel(QListDataModel<QObject*>* m_model_detalle){
	if(idSeccImpresa == "208"){
		m_model_100empresas =  m_model_detalle;
	}else if(idSeccImpresa == "34"){
		m_model_caricaturas =  m_model_detalle;
	}else if(idSeccImpresa == "345128-3"){
		m_model_cartas =  m_model_detalle;
	}else if(idSeccImpresa == "1"){
		m_model_confidenciales =  m_model_detalle;
	}else if(idSeccImpresa == "6"){
		m_model_cultura =  m_model_detalle;
	}else if(idSeccImpresa == "4"){
		m_model_economia =  m_model_detalle;
	}else if(idSeccImpresa == "2"){
		m_model_enfoque =  m_model_detalle;
	}else if(idSeccImpresa == "8"){
		m_model_gente =  m_model_detalle;
	}else if(idSeccImpresa == "5"){
		m_model_mundo =  m_model_detalle;
	}else if(idSeccImpresa == "3"){
		m_model_nacion =  m_model_detalle;
	}else if(idSeccImpresa == "10"){
		m_model_opinion =  m_model_detalle;
	}else if(idSeccImpresa == "12"){
		m_model_sociedad =  m_model_detalle;
	}else if(idSeccImpresa == "7"){
		m_model_vidamoderna =  m_model_detalle;
	}
}

QListDataModel<QObject*>* MagazineBB10::getCurrentDataModel(){
	if(idSeccImpresa == "208"){
		return m_model_100empresas;
	}else if(idSeccImpresa == "34"){
		return m_model_caricaturas;
	}else if(idSeccImpresa == "345128-3"){
		return m_model_cartas;
	}else if(idSeccImpresa == "1"){
		return m_model_confidenciales;
	}else if(idSeccImpresa == "6"){
		return m_model_cultura;
	}else if(idSeccImpresa == "4"){
		return m_model_economia;
	}else if(idSeccImpresa == "2"){
		return m_model_enfoque;
	}else if(idSeccImpresa == "8"){
		return m_model_gente;
	}else if(idSeccImpresa == "5"){
		return m_model_mundo;
	}else if(idSeccImpresa == "3"){
		return m_model_nacion;
	}else if(idSeccImpresa == "10"){
		return m_model_opinion;
	}else if(idSeccImpresa == "12"){
		return m_model_sociedad;
	}else if(idSeccImpresa == "7"){
		return m_model_vidamoderna;
	}
	return new QListDataModel<QObject*>();
}

void MagazineBB10::DetalleImpreso(QVariantList indice ) {
	qDebug() << "\n\n[ApplicationUI] CargarNoticiaRecomendados: " << indice[0].toInt() << "\n";

	QString label = "listaNoticiasImpresas";

	if (root -> findChild<NavigationPane *>("Impresa")) {
		NavigationPane * navpane = root -> findChild<NavigationPane *>("Impresa");

		if(idSeccImpresa == "34" || idSeccImpresa == "12"){
			label = "listaGaleriaCaricaturas";
			pressSite = qmlDetalleImpreso2->createRootObject<Page>();
		}else
			pressSite = qmlDetalleImpreso->createRootObject<Page>();

		navpane -> push(pressSite);
		ListView * lista = pressPage -> findChild<ListView *>("listaNoticiasSeccionesImpresas");

		if (pressSite -> findChild<ListView *>(label)) {
			qDebug() << "\n\n\nLista encontrada" << "\n\n\n" ;
			ListView * lvCarruselNoticia = pressSite -> findChild<ListView *>(label);
			lvCarruselNoticia -> setDataModel(lista -> dataModel());

			pressSite -> setProperty("indice", indice);
			lvCarruselNoticia -> scrollToItem(indice, 1);

			//for(int i = 0; i < m_model_ultimas -> size() ; i++)
			//qobject_cast<NoticiaHome *>(m_model_ultimas -> value(i)) -> load(true);
		}else {
			qDebug() << "\n\n\nLista no encontrada" << "\n\n\n" ;
		}

	}

}

QString MagazineBB10::getSeccion(){
	Tab* tab = root->activeTab();
	return tab->title();
}


/*Flurry events*/

void MagazineBB10::analyticsEventsBuscar(QString tipo, QString busqueda){
	QString tipoStr;
	fprintf(stderr,"\n\n[Magazine BB10] Evento Buscar en Flurry\n\n");
	Flurry::Map parameters;
	if (tipo=="1")
		tipoStr="Relevancia";
	if (tipo=="2")
		tipoStr="Mas Reciente";
	if (tipo=="3")
		tipoStr="Mas Antiguo";
	parameters["Tipo"] = tipoStr;
	parameters["Palabras buscadas"]=busqueda;
	Flurry::Analytics::LogEvent("Buscar", parameters, false);
}

void MagazineBB10::analyticsEvents(QString tipo){
	fprintf(stderr,"\n\n[Magazine BB10] Evento en Flurry\n");
	Flurry::Analytics::LogEvent(tipo, false);
}

void MagazineBB10::analyticsEventsDetalle(QString evento, QString titulo){
	fprintf(stderr,"\n\n[Magazine BB10] Evento Mostrar Detalle en Flurry %s, %s\n",evento.toStdString().c_str(),titulo.toStdString().c_str());
	Flurry::Map parameters;
	parameters["Item"]=titulo;
	Flurry::Analytics::LogEvent(evento, parameters, false);
}

void MagazineBB10::analyticsEventsImpresa(QString seccion){
	fprintf(stderr,"\n\n[Magazine BB10] Evento Mostrar Detalle en Flurry\n");
	Flurry::Map parameters;
	parameters["Seccion"]=seccion;
	Flurry::Analytics::LogEvent("Edici�n Impresa", parameters, false);
}

void MagazineBB10::analyticsEventsImpresaDet(QString seccion, QString titulo){
	fprintf(stderr,"\n\n[Magazine BB10] Evento Mostrar Detalle en Flurry\n");
	Flurry::Map parameters;
	parameters["Seccion"]=seccion;
	parameters["Titulo"]=titulo;
	Flurry::Analytics::LogEvent("Edici�n Impresa", parameters, false);
}

void MagazineBB10::analyticsEventsCompartir(QString seccion, QString titulo){
	fprintf(stderr,"\n\n[Magazine BB10] Evento Mostrar Detalle en Flurry %s, %s\n",seccion.toStdString().c_str(),titulo.toStdString().c_str());
	Flurry::Map parameters;
	parameters["Seccion"]=seccion;
	parameters["Titulo"]=titulo;
	Flurry::Analytics::LogEvent("Compartir en redes", parameters, false);
}

void MagazineBB10::analyticsEventsNoticia(QString seccion, QString titulo){
	fprintf(stderr,"\n\n[Magazine BB10] Evento Mostrar Detalle en Flurry %s, %s\n",seccion.toStdString().c_str(),titulo.toStdString().c_str());
	Flurry::Map parameters;
	parameters["Seccion"]=seccion;
	parameters["Titulo"]=titulo;
	Flurry::Analytics::LogEvent("Noticia", parameters, false);
}

void MagazineBB10::analyticsEventsNot(QString seccion){
	fprintf(stderr,"\n\n[Magazine BB10] Evento Mostrar Detalle en Flurry %s\n",seccion.toStdString().c_str());
	Flurry::Map parameters;
	parameters["Seccion"]=seccion;
	Flurry::Analytics::LogEvent("Noticia", parameters, false);

}
