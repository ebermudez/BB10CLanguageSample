/**
 * Author: Edgardo Bermúdez
 * This class was modified to be able to be shared online via GitHub. 
 * Video.cpp is the object that contains the whole information about videos.
 * 
 * BlackBerry 10 - C++ and QML
 * 
 */

#include "Video.hpp"

Video::Video() {
	// TODO Auto-generated constructor stub
	id = "";
	idTipoItem = "";
	idSeccion = "";
	anteTitulo = "";
	titulo = "";
	resumen = "";
	fechaPublicacion = "";
	autor = "";
	urlFriendly = "";
	metaDescription = "";
	idTipoMultimedia = "";
	imagenPequenia = "";
}

Video::~Video() {
	// TODO Auto-generated destructor stub
}

QString Video::getAnteTitulo() const {
	return anteTitulo;
}

void Video::setAnteTitulo(QString anteTitulo) {
	this->anteTitulo = anteTitulo;
}

QString Video::getAutor() const {
	return autor;
}

void Video::setAutor(QString autor) {
	this->autor = autor;
}

QString Video::getFechaPublicacion() const {
	return fechaPublicacion;
}

void Video::setFechaPublicacion(QString fechaPublicacion) {
	this->fechaPublicacion = fechaPublicacion;
}

QString Video::getId() const {
	return id;
}

void Video::setId(QString id) {
	this->id = id;
}

QString Video::getIdSeccion() const {
	return idSeccion;
}

void Video::setIdSeccion(QString idSeccion) {
	this->idSeccion = idSeccion;
}

QString Video::getIdTipoItem() const {
	return idTipoItem;
}

void Video::setIdTipoItem(QString idTipoItem) {
	this->idTipoItem = idTipoItem;
}

QString Video::getIdTipoMultimedia() const {
	return idTipoMultimedia;
}

void Video::setIdTipoMultimedia(QString idTipoMultimedia) {
	this->idTipoMultimedia = idTipoMultimedia;
}

QString Video::getImagenPequenia() const {
	return imagenPequenia;
}

void Video::setImagenPequenia(QString imagenPequenia) {
	this->imagenPequenia = imagenPequenia;
}

QString Video::getMetaDescription() const {
	return metaDescription;
}

void Video::setMetaDescription(QString metaDescription) {
	this->metaDescription = metaDescription;
}

QString Video::getResumen() const {
	return resumen;
}

void Video::setResumen(QString resumen) {
	this->resumen = resumen;
}

QString Video::getTitulo() const {
	return titulo;
}

void Video::setTitulo(QString titulo) {
	this->titulo = titulo;
}

QString Video::getUrlFriendly() const {
	return urlFriendly;
}

void Video::setUrlFriendly(QString urlFriendly) {
	this->urlFriendly = urlFriendly;
}

void Video::load() {
	AbstractObject::load(true, getImagenPequenia(), getImagenPequenia());
}

QString Video::getContenido() const {
	return contenido;
}

void Video::setContenido(QString contenido) {
	this->contenido = contenido;
}

QString Video::getIdEdicion() const {
	return idEdicion;
}

void Video::setIdEdicion(QString idEdicion) {
	this->idEdicion = idEdicion;
}

QString Video::getUrlMedia() const {
	return urlMedia;
}

void Video::setUrlMedia(QString urlMedia) {
	this->urlMedia = urlMedia;
}

