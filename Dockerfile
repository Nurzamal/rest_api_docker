FROM php:7.1.8-apache
 
ADD . /var/www
COPY .docker/vhost.conf /etc/apache2/sites-available/000-default.conf

RUN chown -R www-data:www-data /var/www \
    && a2enmod rewrite \
    && docker-php-ext-install mbstring pdo pdo_mysql
