/**
 * Author: Edgardo Bermúdez
 * This class was modified to be able to be shared online via GitHub. 
 * This is the object that contains the whole information about videos. Header file.
 * 
 * BlackBerry 10 - C++ and QML
 * 
 */

#ifndef VIDEO_HPP_
#define VIDEO_HPP_

#include "AbstractObject.hpp"

class Video : public AbstractObject {

	Q_OBJECT

	Q_PROPERTY(QString getAnteTitulo READ getAnteTitulo NOTIFY textChanged)
	Q_PROPERTY(QString getAutor READ getAutor NOTIFY textChanged)
	Q_PROPERTY(QString getFechaPublicacion READ getFechaPublicacion NOTIFY textChanged)
	Q_PROPERTY(QString getId READ getId NOTIFY textChanged)
	Q_PROPERTY(QString getIdSeccion READ getIdSeccion NOTIFY textChanged)
	Q_PROPERTY(QString getIdTipoItem READ getIdTipoItem NOTIFY textChanged)
	Q_PROPERTY(QString getIdTipoMultimedia READ getIdTipoMultimedia NOTIFY textChanged)
	Q_PROPERTY(QString getImagenPequenia READ getImagenPequenia NOTIFY textChanged)
	Q_PROPERTY(QString getMetaDescription READ getMetaDescription NOTIFY textChanged)
	Q_PROPERTY(QString getResumen READ getResumen NOTIFY textChanged)
	Q_PROPERTY(QString getTitulo READ getTitulo NOTIFY textChanged)
	Q_PROPERTY(QString getUrlFriendly READ getUrlFriendly NOTIFY textChanged)

	Q_PROPERTY(QString getContenido READ getContenido NOTIFY textChanged)
	Q_PROPERTY(QString getIdEdicion READ getIdEdicion NOTIFY textChanged)
	Q_PROPERTY(QString getUrlMedia READ getUrlMedia NOTIFY textChanged)

public:
	Video();
	virtual ~Video();
	QString getAnteTitulo() const;
	void setAnteTitulo(QString anteTitulo);
	QString getAutor() const;
	void setAutor(QString autor);
	QString getFechaPublicacion() const;
	void setFechaPublicacion(QString fechaPublicacion);
	QString getId() const;
	void setId(QString id);
	QString getIdSeccion() const;
	void setIdSeccion(QString idSeccion);
	QString getIdTipoItem() const;
	void setIdTipoItem(QString idTipoItem);
	QString getIdTipoMultimedia() const;
	void setIdTipoMultimedia(QString idTipoMultimedia);
	QString getImagenPequenia() const;
	void setImagenPequenia(QString imagenPequenia);
	QString getMetaDescription() const;
	void setMetaDescription(QString metaDescription);
	QString getResumen() const;
	void setResumen(QString resumen);
	QString getTitulo() const;
	void setTitulo(QString titulo);
	QString getUrlFriendly() const;
	void setUrlFriendly(QString urlFriendly);
	Q_INVOKABLE void load();

	QString getContenido() const;
	void setContenido(QString contenido);
	QString getIdEdicion() const;
	void setIdEdicion(QString idEdicion);
	QString getUrlMedia() const;
	void setUrlMedia(QString urlMedia);

	Q_SIGNALS:
	void textChanged();

private:
	QString id;
	QString idTipoItem;
	QString idSeccion;
	QString anteTitulo;
	QString titulo;
	QString resumen;
	QString fechaPublicacion;
	QString autor;
	QString urlFriendly;
	QString metaDescription;
	QString idTipoMultimedia;
	QString imagenPequenia;

	// Variables de video individual
	QString idEdicion;
	QString contenido;
	QString urlMedia;

};

#endif /* VIDEO_HPP_ */
