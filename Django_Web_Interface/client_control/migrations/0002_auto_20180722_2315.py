# Generated by Django 2.0.7 on 2018-07-22 23:15

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('client_control', '0001_initial'),
    ]

    operations = [
        migrations.CreateModel(
            name='Switch',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('name', models.CharField(max_length=100)),
                ('type', models.CharField(choices=[('T', 'Toggle'), ('D', 'Dimmer')], max_length=1)),
            ],
        ),
        migrations.RemoveField(
            model_name='pet',
            name='vaccinations',
        ),
        migrations.DeleteModel(
            name='Pet',
        ),
        migrations.DeleteModel(
            name='Vaccine',
        ),
    ]
