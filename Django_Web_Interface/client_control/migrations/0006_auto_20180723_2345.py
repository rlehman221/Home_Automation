# Generated by Django 2.0.7 on 2018-07-23 23:45

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('client_control', '0005_auto_20180723_0047'),
    ]

    operations = [
        migrations.AlterField(
            model_name='switch',
            name='type',
            field=models.CharField(choices=[('On', 'Light On'), ('Off', 'Light Off')], max_length=1),
        ),
    ]