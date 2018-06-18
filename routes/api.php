<?php

use Illuminate\Http\Request;

/*
|--------------------------------------------------------------------------
| API Routes
|--------------------------------------------------------------------------
|
| Here is where you can register API routes for your application. These
| routes are loaded by the RouteServiceProvider within a group which
| is assigned the "api" middleware group. Enjoy building your API!
|
*/

Route::post('register', 'Auth\RegisterController@register');
Route::post('login', 'Auth\LoginController@login');
Route::post('logout', 'Auth\LoginController@logout');
Route::get('tasks', 'JobController@index');
Route::get('tasks/{task}', 'JobController@show');
Route::group(['middleware' => 'auth:api'], function() {
    Route::group(['middleware' => 'freelancer'], function() {
        Route::post('tasks/{task}/take', 'DistributionController@store');
        Route::put('tasks/{task}/finish', 'DistributionController@update');
    });
    
    Route::group(['middleware' => 'role'], function() {
        Route::post('tasks', 'JobController@store');
        Route::put('tasks/{task}', 'JobController@update');
        Route::delete('tasks/{task}', 'JobController@delete');
        Route::get('tasks/{task}/users', 'DistributionController@show');
        
        Route::post('tasks/{task}/{user}', 'TaskAppointController@store');
        Route::put('tasks/{task}/approve', 'TaskAppointController@update');
    });
    
});
