<?php
namespace App;
use Illuminate\Notifications\Notifiable;
use Illuminate\Foundation\Auth\User as Authenticatable;
class User extends Authenticatable
{
    use Notifiable;
    /**
     * Атрибуты, которые можно назначить в массиве.
     *
     * @var array
     */
    protected $fillable = [
        'name', 'email', 'password', 'role',
    ];
    /**
     * 
     *Атрибуты, которые должны быть скрыты для массивов.
     *
     * @var array
     */
    protected $hidden = [
        'password', 'remember_token',
    ];
    public function generateToken()
    {
        $this->api_token = str_random(60);
        $this->save();
        return $this->api_token;
    }
    public function tasks()
    {
        return $this->belongsToMany('App\Task');
    }
}